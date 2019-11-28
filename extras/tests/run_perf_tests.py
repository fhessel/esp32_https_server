#!/usr/bin/env python
import argparse
import os
import socket
import ssl
import sys
import threading
import time
import traceback
import queue
from http import client

parser = argparse.ArgumentParser(description='Run performance tests')
parser.add_argument('--count', default=100, type=int,
  help='Number of test iterations (default: %(default)d)')
parser.add_argument('--host', default='esp.local', type=str,
  help='Hostname or IP of the ESP32 (default: %(default)s)')
parser.add_argument('--http', action='store_false', dest='https',
  help='Use HTTP instead of HTTPS (you may also want to use --port 80)')
parser.add_argument('--poolcount', default=1, type=int,
  help='Number of connection pools (default: %(default)d)')
parser.add_argument('--poolsize', default=2, type=int,
  help='Size of the connection pools (default: %(default)d)')
parser.add_argument('--port', default=443, type=int,
  help='Port on the ESP32 (default: %(default)d)')
parser.add_argument('--reqcount', default=16, type=int,
  help='Requests per connection pool (default: %(default)d)')
parser.add_argument('--resources', default='resourcelist/cats.txt', type=str,
  help='Text file with resources used for the request, one per line (default: %(default)s)')
parser.add_argument('--retries', default=5, type=int,
  help='Retry count for each resource (default: %(default)d)')
parser.add_argument('--timeout', default=30, type=int,
  help='Default connection timeout in seconds (default: %(default)d)')
args = parser.parse_args()

def curtime():
  return int(time.time()*1000000)

start = curtime()

def log(x, end='\n', time=False):
  if time:
    print("%10d %s" % (curtime()-start, x), end=end, file=sys.stderr)
  else:
    print(x, end=end, file=sys.stderr)

class Connection:
  def __init__(self, host, port, id=0, https=True, timeout=30, retries=5, name=None):
    self._id = id
    self._name = "Connection %d" % id if name is None else name
    self._max_retries = retries
    if https:
      ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
      ctx.verify_mode = ssl.CERT_NONE
      ctx.check_hostname = False
      self._con = client.HTTPSConnection(host, port, context=ctx, timeout=timeout)
    else:
      self._con = client.HTTPConnection(host, port, timeout=timeout)
    self._connected = False
    self._stats = {
      'constart': -1,
      'conready': -1,
      'requests': []
    }
    self._reqqueue = queue.Queue(1)
    self._reqthread = threading.Thread(target=self._thread, name=name)
    self._reqthread.start()

  @property
  def id(self):
    return self._id

  def close(self):
    """ Closes the connection, but leaves the connection object alive """
    try:
      log("%s: Closing connection" % self._name, time=True)
      self._con.close()
    except:
      pass
    finally:
      self._connected = False

  def destroy(self):
    """ Destroy the connection instance, closing all underlying connections and stopping threads """
    if self._connected:
      try:
        self._con.close()
      except:
        log("Could not close a connection.")
        traceback.print_exc(file=sys.stderr)
    self._connected = False
    self._reqqueue.put((None,None,None))
    if self._reqthread is not None:
      self._reqthread.join()

  def _thread(self):
    running = True
    log("%s: Thread started" % self._name, time=True)
    retries = 0
    stats = {
      # Number of retries
      "retries": 0,
      # Whether the resouce was successfully received
      "success": False,
      # Time required to create a connection, if a connection hat to be established before
      "time_connect": -1,
      # Time for sending the request and receiving the response
      "time_data": 0,
      # The requested resource
      "resource": "",
    }
    while running:
      method, path, reqdonecb = self._reqqueue.get()
      if method is None:
        running = False
        continue
      log("%s: Handling %s %s" % (self._name, method, path), time=True)
      response = {
        "body": None,
        "size": 0,
        "status": 0,
      }
      stats['retries'] = retries
      if retries < self._max_retries:
        try:
          tstart = curtime()
          if not self._connected:
            self._con.connect()
            self._connected = True
            tcon = curtime()
            stats['time_connect'] = tcon-tstart
            tstart=tcon
          self._con.request(method, path, headers={"Connection":"Keep-Alive"})
          res = self._con.getresponse()
          response['body'] = res.read()
          tres = curtime()
          stats['time_data'] = tres-tstart
          retries = 0
          log("%s: %s %s -> %03d %s" % (self._name, method, path, res.status, res.reason), time=True)
          response['size'] = len(response['body'])
          response['status'] = res.status
          stats['success'] = True
          stats['resource'] = path
        except client.RemoteDisconnected:
          self._connected = False
          retries += 1
          log("%s: Server disconnected meanwhile, retrying (%d of %d)" %
            (self._name, retries, self._max_retries), time=True)
          self._reqqueue.put((method, path, reqdonecb))
          continue
        except client.ImproperConnectionState as e:
          retries += 1
          log("%s: Improper connection state (%s), retrying (%d of %d)" %
            (self._name, e, retries, self._max_retries), time=True)
          self._con.close()
          self._connected = False
          self._reqqueue.put((method, path, reqdonecb))
          continue
        except socket.timeout:
          retries += 1
          log("%s: Timeout, retrying (%d of %d)" %
            (self._name, retries, self._max_retries), time=True)
          self._con.close()
          self._connected = False
          self._reqqueue.put((method, path, reqdonecb))
          continue
        except ConnectionError as e:
          retries += 1
          log("%s: ConnectionError (%s), retrying (%d of %d)" %
            (self._name, e, retries, self._max_retries), time=True)
          self._connected = False
          self._reqqueue.put((method, path, reqdonecb))
          continue
        except:
          retries += 1
          log("%s: Unexpected error, retrying (%d of %d)" %
            (self._name, retries, self._max_retries), time=True)
          traceback.print_exc(file=sys.stderr)
          self._con.close()
          self._connected = False
          self._reqqueue.put((method, path, reqdonecb))
          continue
      else:
        log("%s: Retry limit reached, request failed." % self._name, time=True)
        stats['success'] = False

      if reqdonecb is not None:
        reqdonecb(response, stats, self)
      stats =  {
      "retries": 0,
      "success": False,
      "time_connect": -1,
      "time_data": 0,
      "resource": ""
    }
    log("%s: Thread stopped" % self._name, time=True)

  def request(self, method, path, reqdonecb = None):
    """
    Enqueues a request on this connection. Once it is finished, the callback gets called.

    :param method: The method to use
    :param path: The path to request
    :param reqdoneccb: The (optional) callback to call once the request has finished.
    """
    log("%s: Queueing %s %s" % (self._name, method, path), time=True)
    self._reqqueue.put((method, path, reqdonecb), True)

class ConnectionPool:
  def __init__(self, host, port, size=2, id=0, https=True, retries=5, timeout=30, name=None):
    self._size = size
    self._id = id
    self._name = "Pool %d" % id if name is None else name
    # Idling connections. Connections will be put back there once they're ready to process the next request
    self._con_idle = queue.Queue()
    for n in range(size):
      con = Connection(host, port, id=n, https=https, timeout=timeout, retries=retries, name="Pool %d, Con. %d" % (id,n))
      self._con_idle.put(con)
    # Queue for requests
    self._requestqueue = queue.Queue()
    # Lock to avoid concurrent access to the request function before retrieving the response
    self._reqlock = threading.Lock()
    self._poolthread = None
    self._stats = []

  @property
  def id(self):
    return self._id

  def _thread(self):
    running = True
    log("%s: Thread started" % self._name, time=True)
    # Handle requests
    while running:
      method, path = self._requestqueue.get(True)
      if method is None:
        running = False
        continue
      connection = self._con_idle.get(True)
      connection.request(method, path, self._handlereqdone)
    # Close connections once they're done handling requests
    closed_connections = []
    while len(closed_connections)<self._size:
      con = self._con_idle.get(True)
      con.close()
      closed_connections.append(con)
    # Add them to the idle list again
    for con in closed_connections:
      self._con_idle.put(con)
    log("%s: Thread stopped" % self._name, time=True)

  def _handlereqdone(self, response, stats, connection):
    self._stats.append({
      **stats,
      "pool": self.id,
      "connection": connection.id,
      "size": response['size'],
      "status": response['status'],
    })
    self._con_idle.put(connection)

  def request(self, requests):
    """
    Enqueues requests. Call getresponse() or awaitresponse() to get/wait for the result.

    :param requests: A sequence of (method, path) tuples
    """
    self._reqlock.acquire()
    for request in requests:
      self._requestqueue.put(request)
    self._requestqueue.put((None, None))
    # Start thread
    self._poolthread = threading.Thread(target = self._thread, name=self._name)
    self._poolthread.start()

  def getresponse(self):
    """ Returns the responses, or None if they aren't ready yet """
    if not self._reqlock.locked():
      return None
    stats = self._stats
    self._stats = []
    self._reqlock.release()
    return stats

  def awaitresponse(self):
    """ Awaits all responses and returns them """
    log("%s: Awaiting responses" % self._name, time=True)
    self._poolthread.join()
    return self.getresponse()

  def destroy(self):
    """ Destroys the connection pool, tearing down connections and threads """
    active_connections = self._size
    if self._poolthread is not None:
      self._poolthread.join()
    log("%s: Called destroy(), waiting to destroy %d connections" % (self._name,active_connections), time=True)
    while active_connections > 0:
      con = self._con_idle.get(True)
      con.destroy()
      active_connections -= 1
    log("%s: Destroyed" % self._name, time=True)


resources = [r.strip() for r in open(args.resources, 'r').readlines() if len(r.strip())>0 and r[0]!='#']
log("Found %d resources in %s" % (len(resources), args.resources))
log("Testing %d iterations on %s://%s:%d/" % (args.count, 'https' if args.https else 'http', args.host, args.port))

print('"iteration","pool","connection","resource","success","retries","size","status","time_connect","time_data"')

for run in range(args.count):
  log("Round %5d / %5d" % (run+1, args.count))
  log("-------------------")

  log("Creating connection pools...", time=True)
  pools = [ConnectionPool(args.host, args.port, args.poolsize, timeout=args.timeout, id=i,
    https=args.https) for i in range(args.poolcount)]
  log("Pools created.", time=True)

  log("Enqueueing requests...", time=True)
  requests = [('GET', resources[n%len(resources)]) for n in range(args.reqcount)]
  for pool in pools:
    pool.request(requests)

  log("Awaiting responses...", time=True)
  stats = [p.awaitresponse() for p in pools]
  log("Responses complete.", time=True)

  for poolstats in stats:
    for entry in poolstats:
      print('%(run)d,%(pool)d,%(connection)d,"%(resource)s",%(success)d,%(retries)d,%(size)d,%(status)3d,%(time_connect)d,%(time_data)d' % {**entry, "run":run})

  log("Closing connection pools...", time=True)
  [p.destroy() for p in pools]
  log("Pools closed.")

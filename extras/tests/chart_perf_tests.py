#!/usr/bin/env python
import argparse
import csv
import matplotlib.pyplot as plt
import numpy as np
import sys

parser = argparse.ArgumentParser(description='Display results of performance tests')
parser.add_argument('infile', default='-', type=str,
  help='Input file (use \'-\' for standard input, default: standard input)')
parser.add_argument('--charttype', default='avgrestime', type=str,
  help='Chart type to display the results (default: %(default)s, use \'help\' for a list)')

args = parser.parse_args()

def log(x, end='\n'):
    print(x, end=end, file=sys.stderr)

def avgrestime_selector(row):
  """ Create a row for each request based on (pool, connection, reqtime) """
  return [
    int(row['pool']),
    int(row['connection']),
    int(row['time_data']) + 0 if int(row['time_connect'])<0 else int(row['time_connect'])
  ]

def avgrestime_visualizer(data):
  """ Use (pool, connection, reqtime) to display a chart """
  # pool, connection, reqtime
  pool_labels = ["Pool %d" % p for p in sorted(set(d[0] for d in data))]
  con_labels = ["Conn. %d" % c for c in sorted(set(d[1] for d in data))]
  concount = len(con_labels)
  ind = np.arange(concount)
  width = 0.35
  fig, axs = plt.subplots(len(pool_labels), 1, squeeze=False, constrained_layout=True)
  fig.suptitle("Average Time per Request")
  for pool in range(len(pool_labels)):
    plotmeans = []
    plotstd = []
    for con in range(concount):
      condata = [x[2]/1000.0 for x in data if x[0]==pool and x[1]==con]
      plotmeans.append(np.mean(condata))
      plotstd.append(np.std(condata))
    axs[pool,0].bar(ind, plotmeans, width, yerr=plotstd)
    axs[pool,0].set_title(pool_labels[pool])
    axs[pool,0].set_ylabel('Time (ms)')
    axs[pool,0].set_xticks(ind, minor=False)
    axs[pool,0].set_xticklabels(con_labels, fontdict=None, minor=False)

charttypes = {
  "avgrestime": {
    "description": "Average Time per Request",
    "selector": avgrestime_selector,
    "visualizer": avgrestime_visualizer
  }
}

if not args.charttype in charttypes:
  if args.charttype.lower()!='help':
    log("Invalid chart type. ", end="")
  log("The following options are available for --charttype:")
  for charttype in charttypes.keys():
    print("  %s - %s" % (charttype, charttypes[charttype]['description']))
else:
  charttype = charttypes[args.charttype]

  data = []
  with sys.stdin if args.infile=='-' else open(args.infile, 'r') as infile:
    csvdata = csv.DictReader(infile, delimiter=',', quotechar='"')
    for row in csvdata:
      data.append(charttype['selector'](row))

  charttype['visualizer'](data)
  plt.show()

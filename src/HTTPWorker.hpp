#ifndef SRC_HTTPWORKER_HPP_
#define SRC_HTTPWORKER_HPP_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "HTTPConnection.hpp"

namespace httpsserver {

class HTTPServer; // forward declaration

class HTTPWorker {

public:
  HTTPWorker(HTTPServer * server, size_t stackSize, int priority);
  void start();
  bool isRunning();
  
protected:
  // Create the instance flagged as running
  // If constructor fails to start the actual task, 
  // or task has ended due to server shutdown, this will be set to false.
  bool _running = true;

  // FreeRTOS task handle 
  TaskHandle_t _handle = NULL;

  // HTTP(S)Server to which this task is attached
  HTTPServer * _server = NULL;

  /**
   * Worker (FreeRTOS task) main loop
   */
  void run();

  /**
   * Static method to start the worker in separate FreeRTOS task
   */
  static void static_task(void * param);
};

} // end namespace httpsserver

#endif // SRC_HTTPWORKER_HPP_
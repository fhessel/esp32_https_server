#include "HTTPWorker.hpp"
#include "HTTPServer.hpp"

#include "freertos/semphr.h"

namespace httpsserver {

HTTPWorker::HTTPWorker(HTTPServer * server, size_t stackSize, int priority) {
  _server = server;
  BaseType_t taskRes = xTaskCreate(static_task, "HTTPSWorker", stackSize, this, priority, &_handle);
  if (taskRes == pdTRUE) {
    HTTPS_LOGI("Started connection task %p", _handle);
  } else {
    HTTPS_LOGE("Error starting connection task");
    _running = false;
  }
}

bool HTTPWorker::isRunning() {
  return _running;
}

void HTTPWorker::run() {
  // Run while server is running
  while (_server->isRunning()) {

    if (xSemaphoreTake(_server->_selectMutex, 0) == pdTRUE) {
      // One worker task will manage the connections 
      // i.e block on select call
      HTTPS_LOGV("Task %p managing connections", _handle);
      _server->manageConnections(HTTPS_CONNECTION_TIMEOUT);
      xSemaphoreGive(_server->_selectMutex);
    } else { 
      // While others should wait for work from the queue
       HTTPS_LOGV("Task %p waiting for work", _handle);
      _server->doQueuedWork(portMAX_DELAY);
    }

    // Then all tasks complete any remaining work (without waiting)
    while (_server->doQueuedWork(0));

  } // while server->isRunning()

} // HTTPConnectionTask::run;

void HTTPWorker::start() {
  vTaskResume(_handle);
}

void HTTPWorker::static_task(void* param) {
  HTTPWorker * _this = static_cast<HTTPWorker *>(param);

  // Start suspended wait for server to call start() method
  vTaskSuspend(NULL);  

  // Run the worker 
  _this->run();

  HTTPS_LOGI("Shutting down worker task %p", _this->_handle);
  _this->_running = false;
  // Mark the task for deltetion.
  // The FreeRTOS idle task has reposnibilty to cleanup structures and memory
  vTaskDelete(NULL);
} 


} // namespace httpsserver 

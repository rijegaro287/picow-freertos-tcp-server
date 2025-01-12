#include "processing.h"

void processing_task(void *pvParameters) {
  processing_queue_t *queue = (processing_queue_t*)pvParameters;
  uint8_t recv_buffer[queue->buffer_size];

  while (true) {
    if (xQueueReceive(queue->handle, recv_buffer, 0) == pdTRUE) {
      printf("Processing received data from processing_task: %s\n", recv_buffer);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
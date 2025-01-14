#include "processing.h"

void processing_task(void *pvParameters) {
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  queue_handles_t *handles = (queue_handles_t*)pvParameters;

  uint32_t buffer_size = uxQueueGetQueueItemSize(handles->input_queue);
  uint8_t recv_buffer[buffer_size];

  while(true) {
    if(xQueueReceive(handles->input_queue, recv_buffer, 0) == pdTRUE) {
      /* Process data... */
      printf("-- Processing data: %s\n", recv_buffer);
    }
    else {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}
#pragma once

#include "stdio.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "tcp_server.h"

typedef struct _queue_handles_t {
  xQueueHandle input_queue;
  xSemaphoreHandle input_semaphore;
} queue_handles_t;

extern void processing_task(void *pvParameters);

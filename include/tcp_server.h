#pragma once

#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

typedef struct _tcp_server_config {
  const uint16_t port;
  xQueueHandle input_queue;
  xSemaphoreHandle input_semaphore;
} tcp_server_config_t;


extern void tcp_server_task(void *pvParameters);

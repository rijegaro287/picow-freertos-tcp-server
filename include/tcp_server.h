#pragma once

#include "stdio.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"

#include "FreeRTOS.h"
#include "queue.h"

#include <lwip/sockets.h>


typedef struct _processing_queue {
  QueueHandle_t handle;
  uint32_t buffer_size;
} processing_queue_t;

typedef struct _tcp_server_config {
  uint8_t *ssid;
  uint8_t *password;
  uint16_t port;
  processing_queue_t processing_queue;
} tcp_server_config_t;


extern void tcp_server_task(void *pvParameters);
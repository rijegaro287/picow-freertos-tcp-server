#pragma once

#include "stdio.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "tcp_server.h"


extern void processing_task(void *pvParameters);
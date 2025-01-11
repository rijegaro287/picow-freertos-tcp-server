#include <stdio.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"


void task_1(void *pvParameters)
{
    while (true) {
        printf("Task 1\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_2(void *pvParameters)
{
    while (true) {
        printf("Task 2\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


int main()
{
    stdio_init_all();
    
    TaskHandle_t task_1_handle = NULL;
    TaskHandle_t task_2_handle = NULL;

    uint32_t status_1 = xTaskCreate(task_1,
                                    "Task 1",
                                    1024,
                                    NULL,
                                    tskIDLE_PRIORITY,
                                    &task_1_handle);

    uint32_t status_2 = xTaskCreate(task_2,
                                    "Task 2",
                                    1024,
                                    NULL,
                                    tskIDLE_PRIORITY,
                                    &task_2_handle);

    vTaskStartScheduler();

    return 0;
}

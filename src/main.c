#include "FreeRTOS.h"
#include "task.h"

#include "tcp_server.h"
#include "processing.h"


#define PROCESSING_QUEUE_SIZE 3
#define BUFFER_SIZE 128


void noise_task(void *pvParameters) {
	while(true) {
		printf("Noise\n");
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

int main() {
	stdio_init_all();

	tcp_server_config_t tcp_server_config = {
		.ssid = "Familia Gatgens",
		.password = "adita123",
		.port = 4242,
		.processing_queue = {
			.buffer_size = BUFFER_SIZE,
			.handle = xQueueCreate(PROCESSING_QUEUE_SIZE, BUFFER_SIZE)
		}
	};

	TaskHandle_t tcp_server_handle = NULL;
	TaskHandle_t processing_handle = NULL;
	TaskHandle_t noise_handle = NULL;

	xTaskCreate(tcp_server_task, 
						  "tcp server",
							configMINIMAL_STACK_SIZE,
							&tcp_server_config,
							5,
							&tcp_server_handle);

	xTaskCreate(processing_task,
						  "processing",
							configMINIMAL_STACK_SIZE,
							&(tcp_server_config.processing_queue),
							5,
							&processing_handle);

	xTaskCreate(noise_task,
						  "noise",
							configMINIMAL_STACK_SIZE,
							NULL,
							1,
							&noise_handle);

	vTaskCoreAffinitySet(tcp_server_handle, tskNO_AFFINITY);
	vTaskCoreAffinitySet(processing_handle, tskNO_AFFINITY);
	vTaskCoreAffinitySet(noise_handle, tskNO_AFFINITY);

	// vTaskCoreAffinitySet(tcp_server_handle, (1 << 0));
	// vTaskCoreAffinitySet(processing_handle, (1 << 1));
	// vTaskCoreAffinitySet(noise_handle, (1 << 1));

	vTaskStartScheduler();

	return 0;
}

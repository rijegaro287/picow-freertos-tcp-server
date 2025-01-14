#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"

#include "tcp_server.h"
#include "processing.h"

#define NUMBER_OF_HANDLES 6

#define QUEUE_LENGTH 3
#define BUFFER_SIZE 1024

typedef struct _wifi_credentials {
	const char *ssid;
	const char *password;
} wifi_credentials_t;

typedef enum _handle_index {
	CONNECTION_HANDLE,
	SERVER_HANDLE,
	PROCESSING_HANDLE,
	QUEUE_WATCHER_HANDLE,
	MEMORY_WATCHER_HANDLE
} handle_index_t;

static bool watch_memory = false;

static TaskHandle_t task_handles[NUMBER_OF_HANDLES];

static void wifi_connect_task(void *pvParameters) {
	wifi_credentials_t *credentials = (wifi_credentials_t *)pvParameters;

	if(cyw43_arch_init() != PICO_OK) {
    printf("Failed to initialize CYW43_ARCH\n");
		vTaskDelete(NULL);
		return;
  };

  cyw43_arch_enable_sta_mode();

	int32_t status = CYW43_LINK_FAIL;
	while(status != CYW43_LINK_JOIN) {
		printf("Attempting to connect to Network: %s\n", credentials->ssid);
		cyw43_arch_wifi_connect_timeout_ms(credentials->ssid, 
																			 credentials->password,
																			 CYW43_AUTH_WPA2_MIXED_PSK,
																			 10000);
		
		status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
		if(status == CYW43_LINK_JOIN) {
			printf("Connected to Network: %s\n", credentials->ssid);
		}
		else {
			printf("Failed to connect to Network: %s\n", credentials->ssid);
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}

	for(uint32_t i = 0; i < NUMBER_OF_HANDLES; i++) {
		if((i != CONNECTION_HANDLE) && (task_handles[i] != NULL)) {
			xTaskNotifyGive(task_handles[i]);
		}
	}

	vTaskDelete(NULL);
}

static void queue_watcher_task(void *pvParameters) {
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	queue_handles_t *handles = (queue_handles_t *)pvParameters;
	while(true) {
		if(uxQueueSpacesAvailable(handles->input_queue) > 0) {
			xSemaphoreGive(handles->input_semaphore);
		}

		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

static void memory_watcher_task(void *pvParameters) {
	if(!watch_memory) {
		vTaskDelete(NULL);
	}

	queue_handles_t *handles = (queue_handles_t *)pvParameters;
	while(true) {
		printf("==============================================\n");
		printf("Input Queue: %d\n", uxQueueSpacesAvailable(handles->input_queue));
		printf("----------------------------------------------\n");
		printf("Free Heap: %d\n", xPortGetFreeHeapSize());
		printf("Smallest Free Heap: %d\n", xPortGetMinimumEverFreeHeapSize());
		for(uint32_t i = 0; i < NUMBER_OF_HANDLES; i++) {
			if(task_handles[i] != NULL) {
				printf("Stack Max Usage %d: %d\n", i, uxTaskGetStackHighWaterMark(task_handles[i]));
			}
		}
		printf("==============================================\n");
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

int main() {
	queue_handles_t processing_handles = {
		.input_queue = xQueueCreate(QUEUE_LENGTH, BUFFER_SIZE),
		.input_semaphore = xSemaphoreCreateBinary()
	};

	wifi_credentials_t wifi_credentials = {
		.ssid = "ssid",
		.password = "password"
	};

	tcp_server_config_t tcp_server_config = {
		.port = 4242,
		.input_queue = processing_handles.input_queue,
		.input_semaphore = processing_handles.input_semaphore
	};

	stdio_init_all();

	/* Core 0 */
	xTaskCreate(tcp_server_task, 
						  "tcp server",
							(configMINIMAL_STACK_SIZE << 2),
							&tcp_server_config,
							2,
							&(task_handles[SERVER_HANDLE]));
	
	/* Core 1 */
	xTaskCreate(queue_watcher_task,
						  "queue watcher",
							(configMINIMAL_STACK_SIZE >> 1),
							&processing_handles,
							5,
							&(task_handles[QUEUE_WATCHER_HANDLE]));

	xTaskCreate(processing_task,
						  "processing",
							(configMINIMAL_STACK_SIZE << 2),
							&processing_handles,
							1,
							&(task_handles[PROCESSING_HANDLE]));

	/* No Affinity */
	xTaskCreate(wifi_connect_task,
						  "connection to wifi",
							configMINIMAL_STACK_SIZE,
							&wifi_credentials,
							5,
							&(task_handles[CONNECTION_HANDLE]));

	xTaskCreate(memory_watcher_task,
						  "heap watcher",
							configMINIMAL_STACK_SIZE,
							&processing_handles,
							5,
							&(task_handles[MEMORY_WATCHER_HANDLE]));

	vTaskCoreAffinitySet(task_handles[SERVER_HANDLE], (1 << 0));
	vTaskCoreAffinitySet(task_handles[PROCESSING_HANDLE], (1 << 1));
	vTaskCoreAffinitySet(task_handles[QUEUE_WATCHER_HANDLE], (1 << 1));
	vTaskCoreAffinitySet(task_handles[CONNECTION_HANDLE], tskNO_AFFINITY);
	vTaskCoreAffinitySet(task_handles[MEMORY_WATCHER_HANDLE], tskNO_AFFINITY);

	vTaskStartScheduler();

	return 0;
}

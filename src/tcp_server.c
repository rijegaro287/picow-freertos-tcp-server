#include "tcp_server.h"

static void tcp_server_listen(tcp_server_config_t *config) {
	int32_t server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	struct sockaddr_in server_addr = {
		.sin_len = sizeof(struct sockaddr_in),
		.sin_family = AF_INET,
		.sin_addr = 0,
		.sin_port = htons(config->port)
	};

	if(server_socket < 0) {
		printf("Failed to create server socket\n");
    return;
	}

	if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Failed to bind socket\n");
    return;
	}

	if(listen(server_socket, 1) < 0) {
		printf("Unable to listen on socket: error %d\n", errno);
    return;
	}

	ip_addr_t ip_addr = cyw43_state.netif[0].ip_addr;
	printf("====================================================\n");
	printf("Starting server at %s on port %u\n", ip4addr_ntoa(&ip_addr),
                                               ntohs(server_addr.sin_port));
	printf("====================================================\n");

	while (true) {
    struct sockaddr_storage remote_addr;
		socklen_t len = sizeof(remote_addr);

		int32_t conn_sock = accept(server_socket, (struct sockaddr *)&remote_addr, &len);
		if (conn_sock >= 0) {
			uint32_t buffer_size = uxQueueGetQueueItemSize(config->input_queue);
			uint8_t recv_buffer[buffer_size];

			if(recv(conn_sock, recv_buffer, buffer_size, 0) > 0) {
				if(xSemaphoreTake(config->input_semaphore, portMAX_DELAY)) {
					xQueueSend(config->input_queue, recv_buffer, 0);
          send(conn_sock, "1", 1, 0);
				}
				else {
          send(conn_sock, "0", 1, 0);
				}
			}

			close(conn_sock);
		}
		else {
			vTaskDelay(pdMS_TO_TICKS(1));
		}
	}
}

void tcp_server_task(void *pvParameters) {
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	tcp_server_config_t *config = (tcp_server_config_t *)pvParameters;
  while (true) {
		if(config->input_queue != NULL) {
    	tcp_server_listen(config);
		}
		else {
			printf("Input queue is not set\n");
		}
		vTaskDelay(pdMS_TO_TICKS(500));
  }
}

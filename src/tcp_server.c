#include "tcp_server.h"


static err_t tcp_server_init(tcp_server_config_t *config) {
  if(config->processing_queue.buffer_size == 0) {
    printf("Buffer size not set\n");
    return PICO_ERROR_GENERIC;
  }

  if(config->processing_queue.handle == NULL) {
    printf("Processing queue not set\n");
    return PICO_ERROR_GENERIC;
  }

  if(cyw43_arch_init() != PICO_OK) {
    printf("Failed to initialize CYW43_ARCH\n");
    return PICO_ERROR_GENERIC;
  };

  cyw43_arch_enable_sta_mode();
  return PICO_OK;
}

static err_t tcp_server_connect(tcp_server_config_t *config) {
	uint8_t status = CYW43_LINK_FAIL;
	while(status != CYW43_LINK_JOIN) {
		printf("Attempting to connect to Network: %s\n", config->ssid);
		cyw43_arch_wifi_connect_timeout_ms(config->ssid, 
																			 config->password,
																			 CYW43_AUTH_WPA2_MIXED_PSK,
																			 10000);
		
		status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
		if(status == CYW43_LINK_JOIN) {
			printf("Connected to Network: %s\n", config->ssid);
		}
		else {
			printf("Failed to connect to Network: %s\n", config->ssid);
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}

  return PICO_OK;
}

static void tcp_server_listen(tcp_server_config_t *config) {
	int32_t server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	struct sockaddr_in server_addr = {
		.sin_len = sizeof(struct sockaddr_in),
		.sin_family = AF_INET,
		.sin_port = htons(config->port),
		.sin_addr = 0
	};

	if(server_socket < 0) {
		printf("Failed to create socket\n");
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
		printf("Waiting for connection\n");
		
    struct sockaddr_storage remote_addr;
		socklen_t len = sizeof(remote_addr);

		int32_t conn_sock = accept(server_socket, (struct sockaddr *)&remote_addr, &len);
		if (conn_sock >= 0) {
      uint32_t recv_max_size = config->processing_queue.buffer_size;
			uint8_t recv_buffer[recv_max_size];

			if(recv(conn_sock, recv_buffer, recv_max_size, 0) > 0) {
        xQueueSend(config->processing_queue.handle, recv_buffer, 0);
        if (uxQueueSpacesAvailable(config->processing_queue.handle) == 0) {
          printf("Processing queue is full\n");
          send(conn_sock, "1", 1, 0);
        } else {
          send(conn_sock, "0", 1, 0);
        }
			}

			close(conn_sock);
		}
	}
}

void tcp_server_task(void *pvParameters) {
  tcp_server_config_t *config = (tcp_server_config_t *)pvParameters;
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(500));

    if(tcp_server_init(config) != PICO_OK) {
      continue;
    }

    if(tcp_server_connect(config) != PICO_OK) {
      continue;
    }

    tcp_server_listen(config);
  }
  
	vTaskDelete(NULL);
}

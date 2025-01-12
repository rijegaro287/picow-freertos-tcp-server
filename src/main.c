#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"

#include "FreeRTOS.h"
#include "task.h"

#include <lwip/sockets.h>


#define WIFI_SSID "Familia Gatgens"
#define WIFI_PASSWORD "adita123"


void server_task(void *pvParameters) {
	if(cyw43_arch_init() != PICO_OK) {
		printf("Failed to initialize CYW43\n");
		vTaskDelete(NULL);
	};

	cyw43_arch_enable_sta_mode();
	cyw43_arch_enable_ap_mode;

	printf("CYW43 initialized\n");
	
	uint8_t status = CYW43_LINK_FAIL;
	while(status != CYW43_LINK_JOIN) {
		printf("Attempting to connect to Network: %s\n", WIFI_SSID);
		cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, 
																			 WIFI_PASSWORD,
																			 CYW43_AUTH_WPA2_MIXED_PSK,
																			 10000);
		
		status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
		if(status == CYW43_LINK_JOIN) {
			printf("Connected to Network: %s\n", WIFI_SSID);
		}
		else {
			printf("Failed to connect to Network: %s\n", WIFI_SSID);
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}

	int32_t server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	struct sockaddr_in server_addr = {
		.sin_len = sizeof(struct sockaddr_in),
		.sin_family = AF_INET,
		.sin_port = htons(8080),
		.sin_addr = 0
	};

	if(server_socket < 0) {
		printf("Failed to create socket\n");
	}

	if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Failed to bind socket\n");
	}

	if(listen(server_socket, 1) < 0) {
		printf("Unable to listen on socket: error %d\n", errno);
	}

	uint8_t mac[6];
	ip_addr_t ip_addr = cyw43_state.netif[0].ip_addr;
	cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);
	printf("====================================================\n");
	printf("Starting server at %s on port %u\n", ip4addr_ntoa(&ip_addr), ntohs(server_addr.sin_port));
	printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	printf("====================================================\n");

	while (true) {
		printf("Waiting for connection\n");
		struct sockaddr_storage remote_addr;
		socklen_t len = sizeof(remote_addr);
		int32_t conn_sock = accept(server_socket, (struct sockaddr *)&remote_addr, &len);
		if (conn_sock >= 0) {
			uint8_t buffer[128];
			uint32_t recv_len = recv(conn_sock, buffer, sizeof(buffer), 0);
			buffer[recv_len] = '\0';

			if (recv_len > 0) {
				printf("====================================\n");
				printf("Received %d bytes\n", recv_len);
				printf("Data: %s\n", buffer);
				send(conn_sock, buffer, recv_len, 0);
				printf("Sent %d bytes\n", recv_len);
				printf("====================================\n");
			}

			close(conn_sock);
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	
	cyw43_arch_deinit();
	vTaskDelete(NULL);
}


void noise_task(void *pvParameters) {
	while(true) {
		printf("Noise\n");
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

int main() {
	stdio_init_all();

	TaskHandle_t server_handle = NULL;
	TaskHandle_t noise_handle = NULL;

	xTaskCreate(server_task, "server", configMINIMAL_STACK_SIZE, NULL, 5, &server_handle);
	xTaskCreate(noise_task, "noise", configMINIMAL_STACK_SIZE, NULL, 1, &noise_handle);

	vTaskCoreAffinitySet(server_handle, tskNO_AFFINITY);
	vTaskCoreAffinitySet(noise_handle, tskNO_AFFINITY);

	// vTaskCoreAffinitySet(server_handle, (1 << 0));
	// vTaskCoreAffinitySet(noise_handle, (1 << 1));
	
	vTaskStartScheduler();

	return 0;
}

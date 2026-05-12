#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "vl53l8cx_api.h"

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <i2c_bus> <udp_port>\n", argv[0]);
        printf("Example: %s /dev/i2c-1 5005\n", argv[0]);
        return 1;
    }

    char *i2c_device = argv[1];
    int udp_port = atoi(argv[2]);
    int fd;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcast_enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
    
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(udp_port);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255"); 

    if ((fd = open(i2c_device, O_RDWR)) < 0) {
        printf("Error: Failed to open %s.\n", i2c_device);
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, 0x29) < 0) {
        printf("Error: Sensor on %s is not communicating.\n", i2c_device);
        return 1;
    }

    VL53L8CX_Configuration Dev;
    Dev.platform.fd = fd;

    printf("Initializing sensor on %s (UDP Port: %d)...\n", i2c_device, udp_port);
    if (vl53l8cx_init(&Dev) != 0) {
        printf("Critical error during initialization on %s!\n", i2c_device);
        return 1;
    }

    vl53l8cx_set_resolution(&Dev, VL53L8CX_RESOLUTION_8X8);
    vl53l8cx_set_ranging_frequency_hz(&Dev, 15);
    vl53l8cx_start_ranging(&Dev);

    VL53L8CX_ResultsData Results;
    uint8_t isReady = 0;
    char buffer[1024];

    printf("Sensor is running! Full matrix flipping is active.\n");

    while(1) {
        vl53l8cx_check_data_ready(&Dev, &isReady);
        if (isReady) {
            vl53l8cx_get_ranging_data(&Dev, &Results);
            
            memset(buffer, 0, sizeof(buffer));
            int pos = 0;
            
            for (int row = 7; row >= 0; row--) {
                for (int col = 7; col >= 0; col--) {
                    int idx = (row * 8) + col;
                    uint8_t status = Results.target_status[idx];
                    int dist = Results.distance_mm[idx];
                    
                    if (status == 4 || status == 5 || status == 6 || 
                        status == 8 || status == 9 || status == 10 || 
                        status == 12 || status == 13) {
                        
                        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%d,", dist);

                    } else {
                        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "-1,");
                    }
                }
            }
            
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            
        } else {
            usleep(1000);
        }
    }
    
    return 0;
}
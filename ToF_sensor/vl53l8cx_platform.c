/**
 ******************************************************************************
 * @file    vl53l8cx_platform.cpp
 * @author  STMicroelectronics
 * @version V1.0.0
 * @date    11 November 2021
 * @brief   Implementation of the platform dependent APIs.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2021 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */


#include "vl53l8cx_platform.h"
#include <unistd.h>
#include <stdlib.h>

uint8_t VL53L8CX_RdByte(VL53L8CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t *p_value) {
    uint8_t msg[2] = { RegisterAdress >> 8, RegisterAdress & 0xFF };
    if (write(p_platform->fd, msg, 2) != 2) return 255;
    if (read(p_platform->fd, p_value, 1) != 1) return 255;
    return 0;
}

uint8_t VL53L8CX_WrByte(VL53L8CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t value) {
    uint8_t msg[3] = { RegisterAdress >> 8, RegisterAdress & 0xFF, value };
    if (write(p_platform->fd, msg, 3) != 3) return 255;
    return 0;
}

uint8_t VL53L8CX_RdMulti(VL53L8CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t *p_values, uint32_t size) {
    uint8_t msg[2] = { RegisterAdress >> 8, RegisterAdress & 0xFF };
    if (write(p_platform->fd, msg, 2) != 2) return 255;
    if (read(p_platform->fd, p_values, size) != size) return 255;
    return 0;
}

uint8_t VL53L8CX_WrMulti(VL53L8CX_Platform *p_platform, uint16_t RegisterAdress, uint8_t *p_values, uint32_t size) {
    uint32_t chunk_size = 1024;
    uint32_t position = 0;
    while (position < size) {
        uint32_t current_len = (size - position > chunk_size) ? chunk_size : (size - position);
        uint8_t *buf = malloc(current_len + 2);
        uint16_t current_addr = RegisterAdress + position;
        buf[0] = current_addr >> 8;
        buf[1] = current_addr & 0xFF;
        for (uint32_t i = 0; i < current_len; i++) buf[2 + i] = p_values[position + i];
        
        if (write(p_platform->fd, buf, current_len + 2) != current_len + 2) {
            free(buf);
            return 255;
        }
        free(buf);
        position += current_len;
    }
    return 0;
}

uint8_t VL53L8CX_Reset_Sensor(VL53L8CX_Platform *p_platform) { return 0; }

uint8_t VL53L8CX_WaitMs(VL53L8CX_Platform *p_platform, uint32_t TimeMs) { 
    usleep(TimeMs * 1000); 
    return 0;
}

void VL53L8CX_SwapBuffer(uint8_t *buffer, uint16_t size) {
    uint32_t i;
    uint8_t tmp;
    for(i = 0; i < size; i = i + 4) {
        tmp = buffer[i];
        buffer[i] = buffer[i+3];
        buffer[i+3] = tmp;
        tmp = buffer[i+1];
        buffer[i+1] = buffer[i+2];
        buffer[i+2] = tmp;
    }
}

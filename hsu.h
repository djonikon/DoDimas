#ifndef HSU_H
#define HSU_H

#ifdef __cplusplus
extern "C" {
#endif
    
#define ACK_ERROR 0x10
#define NACK      0x20
#define ACK_OK    0x30
    
#define PREAMBLE_ERROR 0x10
#define LCS_ERROR      0x20
#define SIZE_ERROR     0x30
#define CMD_ERROR      0x40
#define DCS_ERROR      0x50

uint8_t *create_frame(uint8_t *data, uint8_t size);
uint8_t read_ack(UART_HandleTypeDef *huart, uint16_t timeout);
uint8_t *read_responce(UART_HandleTypeDef *huart, uint8_t cmd, uint8_t size, uint16_t timeout);

#ifdef __cplusplus
}
#endif

#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include "hsu_pn.h"


/* Functions -----------------------------------------------------------------*/
//******************************************************************************
uint8_t *create_frame(uint8_t *data, uint8_t *size)
{
    const uint8_t FRAME_SIZE = 20;
    const uint8_t SHIFT = 6;
    static uint8_t frame[FRAME_SIZE];
    uint8_t sum = 0xD4;

    frame[0] = 0x00;          // Preamble
    frame[1] = 0x00;          // Start of Packet Code
    frame[2] = 0xFF; 
    frame[3] = *size + 1;      // LEN -> Packet Length (TFI and PD0 to PDn)
    frame[4] = ~frame[3] + 1; // LCS -> Packet Length Checksum
    frame[5] = 0xD4;          // TFI -> frame from the controller to the PN532

    for (uint8_t i=0; i<*size; i++)
    { 
        frame[i+SHIFT] = *data;
        sum += *data;
        data++;
    }

    frame[SHIFT+*size] = ~sum + 1; // DCS -> Data Checksum (TFI and PD0 to PDn)
    frame[SHIFT+*size+1] = 0x00;   // Postamble
 
		*size = SHIFT+*size+2;
		
    return frame;
}


//******************************************************************************
uint8_t read_ack(UART_HandleTypeDef *huart, uint16_t timeout)
{
    uint8_t FRAME_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    uint8_t FRAME_NACK[] = {0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    
    uint8_t buffer[6];
		
    HAL_UART_Receive(huart, buffer, 6, timeout);
    
    for (uint8_t i=0; i<6; i++)
    {
        if (buffer[i] != FRAME_ACK[i])
        {
            break;
        }
        else if (i == 5)
        {
            return ACK_OK;
        }
    }
		
    for (uint8_t i=0; i<6; i++)
    {
        if (buffer[i] != FRAME_NACK[i])
        {
            return ACK_ERROR;
        }
        else if (i == 5)
        {
            return NACK;
        }
    }
    return ACK_ERROR;
}


//******************************************************************************
uint8_t *read_responce(UART_HandleTypeDef *huart, uint8_t cmd, uint8_t size, uint16_t timeout)
{
    const uint8_t FRAME_SIZE = 20; // Определиться с размером
    // Первый элемент - размер данных, следующие элементы - сами данные
    static uint8_t data[FRAME_SIZE];
    uint8_t buffer[3] = {0x00, 0x00, 0x00};
    
    HAL_UART_Receive(huart, buffer, sizeof(buffer), timeout);
    if ((buffer[0] != 0x00) || (buffer[1] != 0x00) || (buffer[2] != 0xFF)) 
    {
        data[0] = 0x00;
        data[1] = PREAMBLE_ERROR;
        return data;
    }
    
    uint8_t length[2];
    HAL_UART_Receive(huart, length, sizeof(length), timeout);
    if ((length[0] + length[1]) != 0x00)
    {
        data[0] = 0x00;
        data[1] = LCS_ERROR;
        return data;
    }
    
    length[0] -= 1; // - TFI, - PD0, + length (data[0])
    if (length[0] > size)
    {
        data[0] = 0x00;
        data[1] = SIZE_ERROR;
        return data;
    }
    
    uint8_t command = cmd + 1;
    HAL_UART_Receive(huart, buffer, 2, timeout);
    if ((buffer[0] != 0xD5) || (buffer[1] != command))
    {
        data[0] = 0x00;
        data[1] = CMD_ERROR;
        return data;
    }
    
    HAL_UART_Receive(huart, &data[1], size, timeout);
    data[0] = length[0];
    
    uint8_t sum = 0xD5 + command;
    for (uint8_t i=0; i<length[0]; i++)
    {
        sum += data[i+1];
    }
    
    HAL_UART_Receive(huart, buffer, 2, timeout);
    if (((sum + buffer[0]) != 0x00) || (buffer[1] != 0x00))
    {
        data[0] = 0x00;
        data[1] = DCS_ERROR;
        return data;
    }
    
    return data;
}

//******************************************************************************

void pn_wakeup (UART_HandleTypeDef *huart) {
		
		uint8_t WAKEUP[] = {0x55, 0x55, 0x00, 0x00};
		HAL_UART_Transmit(huart, WAKEUP, 4, 1);
}

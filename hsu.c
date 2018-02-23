/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"


/* Functions -----------------------------------------------------------------*/
//******************************************************************************
uint8_t *create_frame(uint8_t *data, uint8_t size)
{
    const uint8_t FRAME_SIZE = 8 + sizeof(data);
    const uint8_t SHIFT = 6;
    static uint8_t frame[FRAME_SIZE];
    uint8_t sum = 0xD4;

    frame[0] = 0x00;          // Preamble
    frame[1] = 0x00;          // Start of Packet Code
    frame[2] = 0xFF; 
    frame[3] = size + 1;      // LEN -> Packet Length (TFI and PD0 to PDn)
    frame[4] = ~frame[3] + 1; // LCS -> Packet Length Checksum
    frame[5] = 0xD4;          // TFI -> frame from the controller to the PN532

    for (uint8_t i=0; i<size; i++)
    { 
        frame[i+SHIFT] = *data;
        sum += *data;
        data++;
    }

    frame[SHIFT+size] = ~sum + 1; // DCS -> Data Checksum (TFI and PD0 to PDn)
    frame[SHIFT+size+1] = 0x00;   // Postamble
 
    return frame;
}

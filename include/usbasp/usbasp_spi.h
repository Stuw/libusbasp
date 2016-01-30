#ifndef USBASP_SPI_H
#define USBASP_SPI_H

#include <stdint.h>

#define USBASP_SPI_BIT_ORDER_MSBFIRST 1
#define USBASP_SPI_MODE0 0
#define USBASP_SPI_CS0 0 
#define LOW 0
#define HIGH 1

#define usbasp_gpio_fsel(gpio, fsel)
#define usbasp_gpio_write(gpio, level)

void usbasp_set_debug(int n);

int usbasp_init(void);
int usbasp_close(void);
void usbasp_spi_begin(void);
void usbasp_spi_end(void);
void usbasp_spi_setBitOrder(uint8_t order);
void usbasp_spi_setDataMode(uint8_t mode);
void usbasp_spi_setClockDivider(uint16_t divider);
void usbasp_spi_chipSelect(uint8_t cs);
void usbasp_spi_setChipSelectPolarity(uint8_t cs, uint8_t active);

void usbasp_spi_nss(int nss);
uint8_t usbasp_spi_send(uint8_t value);

uint8_t usbasp_spi_transfer(uint8_t value);
void usbasp_spi_transfernb(char* tbuf, char* rbuf, uint32_t len);
void usbasp_spi_transfern(char* buf, uint32_t len);

#endif // USBASP_SPI_H

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <unistd.h>

#include <usbasp/usbasp_spi.h>


int send();

int main(void)
{
	usbasp_set_debug(2);

	int res = usbasp_init();
	if (res)
		return res;

	send();

	usbasp_close();

	return 0;
}

#define MFC522_READ (1 << 7)
#define MFC522_READ_REG(x) (MFC522_READ | (x << 1))

int send()
{
	uint32_t v;

	usbasp_spi_begin();
	usleep(50000);

	v = 0x0f02; usbasp_spi_transfern((char*)&v, 2);
	usleep(10000);
	v = 0x00a8; usbasp_spi_transfern((char*)&v, 2);
	v = 0x8028; usbasp_spi_transfern((char*)&v, 2);
	usleep(10000);
	v = 0x14a8; usbasp_spi_transfern((char*)&v, 2);

	usbasp_spi_end();
	
	return 0;
}



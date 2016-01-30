#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#include <usbasp/usbasp.h>
#include <usbasp/usbasp_spi.h>

const uint16_t g_vid = 0x16c0;
const uint16_t g_pid = 0x05dc;
volatile int g_debug = 0;

struct libusb_context *g_ctx = NULL;
struct libusb_device_handle* g_dev = NULL;


const char* usbasp_req_to_str(int req);

int usbasp_req_data(struct libusb_device_handle *dev,
		    int req, uint16_t wValue, uint16_t wIndex,
		    unsigned char *data, int size);
int usbasp_req(struct libusb_device_handle *dev, int req, int val);



void usbasp_set_debug(int n)
{
	g_debug = n;
}


int usbasp_init(void)
{
	int res;
	uint8_t i;
	unsigned char desc_data[256];

	res = libusb_init(&g_ctx);
	if (res)
	{
		fprintf(stderr, "Error: failed to init libusb: %d (%s)\n", res, libusb_error_name(res));
		return res;
	}

	g_dev = libusb_open_device_with_vid_pid(g_ctx, g_vid, g_pid);
	if (!g_dev)
	{
		fprintf(stderr, "Error: failed to open USBASP device (%04x:%04x)\n", g_vid, g_pid);
		return 1;
	}

	for (i = 1; i < 128; ++i)
	{
		res = libusb_get_string_descriptor_ascii(g_dev, i, desc_data, sizeof(desc_data));
		if (res < 0)
			break;

		if (g_debug)
			printf("USB string desc %d: %s\n", i, desc_data);

		continue;
	}

	return 0;
}

int usbasp_close(void)
{
	if (g_dev)
		libusb_close(g_dev);
	if (g_ctx)
		libusb_exit(g_ctx);

	return 0;
}

void usbasp_spi_begin(void)
{
	usbasp_req(g_dev, USBASP_FUNC_CONNECT, 0);
}

void usbasp_spi_end(void)
{
	usbasp_req(g_dev, USBASP_FUNC_DISCONNECT, 0);
}

void usbasp_spi_setBitOrder(uint8_t order)
{
	(void)order;
}
void usbasp_spi_setDataMode(uint8_t mode)
{
	(void)mode;
}
void usbasp_spi_setClockDivider(uint16_t divider)
{
	(void)divider;
}
void usbasp_spi_chipSelect(uint8_t cs)
{
	(void)cs;
}
void usbasp_spi_setChipSelectPolarity(uint8_t cs, uint8_t active)
{
	(void)cs;
	(void)active;
}


void usbasp_spi_transfernb(char* tbuf, char* rbuf, uint32_t len)
{
	uint16_t wValue;
	uint16_t wIndex;

	while (len >= 4)
	{
		wValue = *(uint16_t*)tbuf;
		wIndex = *(uint16_t*)(tbuf + 2);

		usbasp_req_data(g_dev, USBASP_FUNC_TRANSMIT2,
		    wValue, wIndex,
		    (unsigned char *)rbuf, 4);
		tbuf += 4;
		rbuf += 4;
		len -= 4;
	}

	wValue = 0;
	wIndex = 0;
	switch (len)
	{
		case 3:
			wValue = *(uint16_t*)tbuf;
			wIndex = *(uint8_t*)(tbuf + 2);
 			break;
 		case 2:
 			wValue = *(uint16_t*)tbuf;
 			break;
 		case 1:
 			wValue = *(uint8_t*)tbuf;
			break;
		default:
			return;
	}

	usbasp_req_data(g_dev, USBASP_FUNC_TRANSMIT2,
		    wValue, wIndex,
		    (unsigned char *)rbuf, len);
}


uint8_t usbasp_spi_transfer(uint8_t value)
{
	uint8_t rvalue = 0;
	usbasp_spi_transfernb((char*)&value, (char*)&rvalue, 1);
	return rvalue;
}


void usbasp_spi_transfern(char* buf, uint32_t len)
{
	usbasp_spi_transfernb(buf, buf, len);
}


void usbasp_spi_transfernn(uint32_t value)
{
	uint32_t rvalue = 0;
	usbasp_spi_transfernb((char*)&value, (char*)&rvalue, sizeof(value));
	if (g_debug) {
		printf("%02x -> %02x\n", value & 0xff, rvalue & 0xff);
		printf("%02x -> %02x\n", (value >> 8) & 0xff, (rvalue >> 8) & 0xff);
	}
}

void usbasp_spi_nss(int nss)
{
	if (!nss)
		usbasp_req(g_dev, USBASP_FUNC_BEGIN_TRANSFER, 0);
	else
		usbasp_req(g_dev, USBASP_FUNC_END_TRANSFER, 0);
}

uint8_t usbasp_spi_send(uint8_t value)
{
	uint16_t wValue = value;
	uint16_t wIndex = 0;

	usbasp_req_data(g_dev, USBASP_FUNC_TRANSFER1,
		    wValue, wIndex,
		    (unsigned char *)&value, 1);

	return value;
}



int usbasp_req_data(struct libusb_device_handle *dev,
		    int req, uint16_t wValue, uint16_t wIndex,
		    unsigned char *data, int size)
{
	int i, res;

	if (g_debug)
	{
		printf("req %d (%s), data out:", req, usbasp_req_to_str(req));
		for (i = 0; i < size; ++i)
		{
			printf(" 0x%x", data[i]);
		}
		printf("\n");
	}

	res = libusb_control_transfer(dev, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR,
					req, wValue, wIndex,
					data, size,
					5000 /* timeout */);
	if (req < USBASP_FUNC_TRANSMIT2 && res != 0)
	{
		fprintf(stderr, "Error: req %d: %s\n", req, libusb_error_name(res));
		return res;
	}

	if (g_debug)
	{	
		printf("req %d (%s), data in: ", req, usbasp_req_to_str(req));
		for (i = 0; i < size; ++i)
		{
			printf(" 0x%x", data[i]);
		}
		printf("\n");
	}

	return 0;
}

int usbasp_req(struct libusb_device_handle *dev, int req, int val)
{
	unsigned char data[4] = { '\0' };
	uint16_t wValue = val & 0xffff;
	uint16_t wIndex = (val >> 16) & 0xffff;

	return usbasp_req_data(dev, req, wValue, wIndex, data, 4);
}

const char* usbasp_req_to_str(int req)
{
	switch (req)
	{
		case USBASP_FUNC_CONNECT: return "Connect";
		case USBASP_FUNC_DISCONNECT: return "Disconnect";
		case USBASP_FUNC_TRANSMIT: return "Transmit";
		case USBASP_FUNC_TRANSMIT2: return "Transmit2";
		case USBASP_FUNC_READFLASH: return "Read flash";
		case USBASP_FUNC_ENABLEPROG: return "Enable prog";
		case USBASP_FUNC_WRITEFLASH: return "Write flash";
		case USBASP_FUNC_READEEPROM: return "Read eeprom";
		case USBASP_FUNC_WRITEEEPROM: return "Write eeprom";
		case USBASP_FUNC_SETLONGADDRESS: return "Set long addr";
		case USBASP_FUNC_SETISPSCK: return "Set ISP SCK";
		case USBASP_FUNC_TPI_CONNECT: return "TPI connect";
		case USBASP_FUNC_TPI_DISCONNECT: return "TPI disconnect";
		case USBASP_FUNC_TPI_RAWREAD: return "RAW read";
		case USBASP_FUNC_TPI_RAWWRITE: return "RAW write";
		case USBASP_FUNC_TPI_READBLOCK: return "Read block";
		case USBASP_FUNC_TPI_WRITEBLOCK: return "Write block";
		case USBASP_FUNC_GETCAPABILITIES: return "Get capabilities";
	}

	return "<Unknown>";
}


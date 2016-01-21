#include <libusb-1.0/libusb.h>
#include <stdio.h>


int list_devices(libusb_context* ctx);


int main(void)
{
	struct libusb_context *ctx;

	int res = libusb_init(&ctx);

	list_devices(ctx);

	libusb_exit(ctx);

	return res;
}


const char* dev_class_to_str(enum libusb_class_code c)
{
	switch (c)
	{
		case LIBUSB_CLASS_PER_INTERFACE: return "Per interface";
	
		/** Audio class */
		case LIBUSB_CLASS_AUDIO: return "Audio";
	
		/** Communications class */
		case LIBUSB_CLASS_COMM: return "Communiction";
	
		/** Human Interface Device class */
		case LIBUSB_CLASS_HID: return "HID";
	
		/** Physical */
		case LIBUSB_CLASS_PHYSICAL: return "Physical";
	
		/** Printer class */
		case LIBUSB_CLASS_PRINTER: return "Printer";
	
		/** Image class */
		/* LIBUSB_CLASS_IMAGE: */
		case LIBUSB_CLASS_PTP: return "Image/PTP";
	
		/** Mass storage class */
		case LIBUSB_CLASS_MASS_STORAGE: return "Storage";
	
		/** Hub class */
		case LIBUSB_CLASS_HUB: return "Hub";
	
		/** Data class */
		case LIBUSB_CLASS_DATA: return "Data";
	
		/** Smart Card */
		case LIBUSB_CLASS_SMART_CARD: return "Smart Card";
	
		/** Content Security */
		case LIBUSB_CLASS_CONTENT_SECURITY: return "Security";
	
		/** Video */
		case LIBUSB_CLASS_VIDEO: return "Video";
	
		/** Personal Healthcare */
		case LIBUSB_CLASS_PERSONAL_HEALTHCARE: return "Healcare";
	
		/** Diagnostic Device */
		case LIBUSB_CLASS_DIAGNOSTIC_DEVICE: return "Diagnostic device";
	
		/** Wireless class */
		case LIBUSB_CLASS_WIRELESS: return "Wireless";
	
		/** Application class */
		case LIBUSB_CLASS_APPLICATION: return "Application";
	
		/** Class is vendor-specific */
		case LIBUSB_CLASS_VENDOR_SPEC: return "Vendor-specific";
	}	

	return "<Unknown>";
}


const char* transfer_type_to_str(enum libusb_transfer_type type)
{
	switch (type)
	{
		/** Control endpoint */
		case LIBUSB_TRANSFER_TYPE_CONTROL: return "Control";
		/** Isochronous endpoint */
		case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS: return "Isochronous";
		/** Bulk endpoint */
		case LIBUSB_TRANSFER_TYPE_BULK: return "Bulk";
		/** Interrupt endpoint */
		case LIBUSB_TRANSFER_TYPE_INTERRUPT: return "Interrupt";
		/** Stream endpoint */
		case LIBUSB_TRANSFER_TYPE_BULK_STREAM: return "Stream";
	}

	return "<Unknown";
}

int list_endpoints(const struct libusb_interface_descriptor *desc)
{
	int i;
	for (i = 0; i < desc->bNumEndpoints; ++i)
	{
		printf("        Endpoint %d\n", i);
		const struct libusb_endpoint_descriptor *ep = &desc->endpoint[i];
		printf("          Addr 0x%x (0x%x)\n", ep->bEndpointAddress, ep->bEndpointAddress & 0x7f);
		printf("          Direction %s\n", (ep->bEndpointAddress & LIBUSB_ENDPOINT_IN) ? "In" : "Out");
		printf("          Transfer type %d (%s)\n",
						ep->bmAttributes & 0x3,
						transfer_type_to_str(ep->bmAttributes & 0x3));
	}

	return 0;
}


int list_altsettings(const struct libusb_interface *interface)
{
	int i;

	for (i = 0; i < interface->num_altsetting; ++i)
	{
		const struct libusb_interface_descriptor *desc = &interface->altsetting[i];
		printf("      Altsetting %d\n", i);
		printf("        Num %d\n", desc->bInterfaceNumber);
		printf("        Class 0x%02x (%s)\n",
					desc->bInterfaceClass,
					dev_class_to_str(desc->bInterfaceClass));
		printf("        Endpoints %d\n", desc->bNumEndpoints);
		list_endpoints(desc);
	}

	return 0;
}


int list_interfaces(struct libusb_config_descriptor *conf_desc)
{
	ssize_t i;

	for (i = 0; i < conf_desc->bNumInterfaces; ++i)
	{
		printf("    Interface %d\n", i);
		printf("      Altsettings %d\n", conf_desc->interface[i].num_altsetting);
		list_altsettings(&conf_desc->interface[i]);
	}

	return 0;
}


int list_configurations(struct libusb_device* dev,
			struct libusb_device_descriptor *dev_desc)
{
	ssize_t i;
	int res;
	struct libusb_config_descriptor *conf_desc;
	for (i = 0; i < dev_desc->bNumConfigurations; ++i)
	{
		res = libusb_get_config_descriptor(dev, i, &conf_desc);
		if (res < 0) {
			fprintf(stderr, "Error: can't get config desc\n");
		} else {
			printf("  Config %d\n", conf_desc->iConfiguration);
			printf("    Max power %d\n", conf_desc->MaxPower);
			printf("    Interfaces %d\n", conf_desc->bNumInterfaces);
			list_interfaces(conf_desc);
			libusb_free_config_descriptor(conf_desc);
		}
	}

	return 0;
}


int list_devices(struct libusb_context *ctx)
{
	struct libusb_device **devs;
	struct libusb_device_descriptor desc;
	int res;
	ssize_t i;

	ssize_t dev_count = libusb_get_device_list(ctx, &devs);

	if (dev_count < 0)
	{
		fprintf(stderr, "Error: can't get dev list\n");
		return 0;
	}

	for (i = 0; i < dev_count; ++i)
	{
		res = libusb_get_device_descriptor(devs[i], &desc);
		if (res < 0) {
			fprintf(stderr, "Error: failed to get device descriptor\n");
		} else {
			printf("Device %d\n", i);
			printf("  ID %04x:%04x\n", desc.idVendor, desc.idProduct);
			printf("  Class 0x%02x (%s)\n",
					desc.bDeviceClass,
					dev_class_to_str(desc.bDeviceClass));
			printf("  USB %x\n", desc.bcdUSB);
			printf("  Configs %d\n", desc.bNumConfigurations);
			list_configurations(devs[i], &desc);
		}
	}

	libusb_free_device_list(devs, 1);

	return 0;
}

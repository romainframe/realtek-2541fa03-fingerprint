/*
 * Advanced USB Fingerprint Sensor Probe Tool
 *
 * For Realtek/Microctopus MoC (USB ID 2541:fa03)
 *
 * This tool performs deeper USB analysis:
 * - Dumps all USB descriptors
 * - Tests control transfers
 * - Tries interrupt endpoints
 * - Checks for firmware requirements
 *
 * Build: gcc probe_advanced.c -o probe_advanced -lusb-1.0
 * Run: sudo ./probe_advanced
 */

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define VID 0x2541
#define PID 0xfa03

void print_hex(const char *label, const unsigned char *data, int len) {
    printf("%s: ", label);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0 && i < len - 1) {
            printf("\n%*s  ", (int)strlen(label), "");
        }
    }
    printf("\n");
}

void dump_device_descriptor(libusb_device *dev) {
    struct libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret < 0) {
        printf("Failed to get device descriptor\n");
        return;
    }

    printf("\n=== Device Descriptor ===\n");
    printf("bLength: %d\n", desc.bLength);
    printf("bDescriptorType: %d\n", desc.bDescriptorType);
    printf("bcdUSB: %04X\n", desc.bcdUSB);
    printf("bDeviceClass: %d\n", desc.bDeviceClass);
    printf("bDeviceSubClass: %d\n", desc.bDeviceSubClass);
    printf("bDeviceProtocol: %d\n", desc.bDeviceProtocol);
    printf("bMaxPacketSize0: %d\n", desc.bMaxPacketSize0);
    printf("idVendor: %04X\n", desc.idVendor);
    printf("idProduct: %04X\n", desc.idProduct);
    printf("bcdDevice: %04X\n", desc.bcdDevice);
    printf("iManufacturer: %d\n", desc.iManufacturer);
    printf("iProduct: %d\n", desc.iProduct);
    printf("iSerialNumber: %d\n", desc.iSerialNumber);
    printf("bNumConfigurations: %d\n", desc.bNumConfigurations);
}

void dump_config_descriptor(libusb_device *dev) {
    struct libusb_config_descriptor *config;
    int ret = libusb_get_active_config_descriptor(dev, &config);
    if (ret < 0) {
        printf("Failed to get config descriptor\n");
        return;
    }

    printf("\n=== Configuration Descriptor ===\n");
    printf("bNumInterfaces: %d\n", config->bNumInterfaces);
    printf("bConfigurationValue: %d\n", config->bConfigurationValue);
    printf("iConfiguration: %d\n", config->iConfiguration);
    printf("bmAttributes: 0x%02X\n", config->bmAttributes);
    printf("MaxPower: %d mA\n", config->MaxPower * 2);

    for (int i = 0; i < config->bNumInterfaces; i++) {
        const struct libusb_interface *inter = &config->interface[i];
        printf("\n--- Interface %d ---\n", i);

        for (int j = 0; j < inter->num_altsetting; j++) {
            const struct libusb_interface_descriptor *interdesc = &inter->altsetting[j];
            printf("  bInterfaceNumber: %d\n", interdesc->bInterfaceNumber);
            printf("  bAlternateSetting: %d\n", interdesc->bAlternateSetting);
            printf("  bNumEndpoints: %d\n", interdesc->bNumEndpoints);
            printf("  bInterfaceClass: %d\n", interdesc->bInterfaceClass);
            printf("  bInterfaceSubClass: %d\n", interdesc->bInterfaceSubClass);
            printf("  bInterfaceProtocol: %d\n", interdesc->bInterfaceProtocol);

            for (int k = 0; k < interdesc->bNumEndpoints; k++) {
                const struct libusb_endpoint_descriptor *epdesc = &interdesc->endpoint[k];
                printf("    Endpoint %d:\n", k);
                printf("      bEndpointAddress: 0x%02X (%s)\n", epdesc->bEndpointAddress,
                       (epdesc->bEndpointAddress & LIBUSB_ENDPOINT_IN) ? "IN" : "OUT");
                printf("      bmAttributes: 0x%02X (", epdesc->bmAttributes);
                switch (epdesc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) {
                    case LIBUSB_TRANSFER_TYPE_CONTROL: printf("Control"); break;
                    case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS: printf("Isochronous"); break;
                    case LIBUSB_TRANSFER_TYPE_BULK: printf("Bulk"); break;
                    case LIBUSB_TRANSFER_TYPE_INTERRUPT: printf("Interrupt"); break;
                }
                printf(")\n");
                printf("      wMaxPacketSize: %d\n", epdesc->wMaxPacketSize);
                printf("      bInterval: %d\n", epdesc->bInterval);
            }
        }
    }

    libusb_free_config_descriptor(config);
}

int test_control_transfer(libusb_device_handle *handle, const char *name,
                          uint8_t bmRequestType, uint8_t bRequest,
                          uint16_t wValue, uint16_t wIndex, uint16_t wLength) {
    unsigned char data[256];
    memset(data, 0, sizeof(data));

    printf("\n=== Control Transfer: %s ===\n", name);
    printf("bmRequestType: 0x%02X\n", bmRequestType);
    printf("bRequest: 0x%02X\n", bRequest);
    printf("wValue: 0x%04X\n", wValue);
    printf("wIndex: 0x%04X\n", wIndex);
    printf("wLength: %d\n", wLength);

    int ret = libusb_control_transfer(handle, bmRequestType, bRequest,
                                      wValue, wIndex, data, wLength, 1000);

    if (ret < 0) {
        printf("ERROR: %s (%d)\n", libusb_error_name(ret), ret);
        return -1;
    }

    printf("Received %d bytes\n", ret);
    if (ret > 0) {
        print_hex("Data", data, ret);
        return 0;
    }

    return -1;
}

int test_interrupt_endpoint(libusb_device_handle *handle, uint8_t endpoint, const char *name) {
    unsigned char data[256];
    int transferred;

    printf("\n=== Interrupt Endpoint %s (0x%02X) ===\n", name, endpoint);

    int ret = libusb_interrupt_transfer(handle, endpoint, data, sizeof(data),
                                       &transferred, 1000);

    if (ret < 0) {
        printf("ERROR: %s (%d)\n", libusb_error_name(ret), ret);
        return -1;
    }

    printf("Received %d bytes\n", transferred);
    if (transferred > 0) {
        print_hex("Data", data, transferred);
        return 0;
    }

    return -1;
}

int main(void) {
    libusb_context *ctx = NULL;
    libusb_device_handle *handle = NULL;
    libusb_device *dev = NULL;
    int ret;

    printf("Advanced Realtek 2541:fa03 Fingerprint Sensor Probe\n");
    printf("===================================================\n");

    // Initialize libusb
    ret = libusb_init(&ctx);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(ret));
        return 1;
    }

    // Open device
    handle = libusb_open_device_with_vid_pid(ctx, VID, PID);
    if (!handle) {
        fprintf(stderr, "Failed to open device %04X:%04X\n", VID, PID);
        libusb_exit(ctx);
        return 1;
    }

    dev = libusb_get_device(handle);

    printf("Device %04X:%04X opened successfully\n\n", VID, PID);

    // Dump descriptors
    dump_device_descriptor(dev);
    dump_config_descriptor(dev);

    // Get string descriptors
    printf("\n=== String Descriptors ===\n");
    unsigned char string[256];
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(dev, &desc);

    if (desc.iManufacturer) {
        ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, string, sizeof(string));
        if (ret > 0) {
            printf("Manufacturer: %s\n", string);
        }
    }

    if (desc.iProduct) {
        ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string, sizeof(string));
        if (ret > 0) {
            printf("Product: %s\n", string);
        }
    }

    if (desc.iSerialNumber) {
        ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, string, sizeof(string));
        if (ret > 0) {
            printf("Serial: %s\n", string);
        }
    }

    // Detach kernel driver
    if (libusb_kernel_driver_active(handle, 0) == 1) {
        printf("\nDetaching kernel driver...\n");
        libusb_detach_kernel_driver(handle, 0);
    }

    // Claim interface
    ret = libusb_claim_interface(handle, 0);
    if (ret < 0) {
        fprintf(stderr, "Failed to claim interface: %s\n", libusb_error_name(ret));
        libusb_close(handle);
        libusb_exit(ctx);
        return 1;
    }

    printf("\n=== Testing USB Communication ===\n");

    // Test standard USB control requests
    test_control_transfer(handle, "Get Status",
                         LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
                         LIBUSB_REQUEST_GET_STATUS, 0, 0, 2);

    test_control_transfer(handle, "Get Descriptor (Device)",
                         LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE,
                         LIBUSB_REQUEST_GET_DESCRIPTOR, 0x0100, 0, 18);

    // Test vendor-specific control requests
    printf("\n=== Vendor-Specific Control Transfers ===\n");
    for (int i = 0; i < 16; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Vendor Request 0x%02X", i);
        test_control_transfer(handle, name,
                             LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                             i, 0, 0, 64);
        usleep(100000); // 100ms delay
    }

    // Test interrupt endpoints
    printf("\n=== Testing Interrupt Endpoints ===\n");
    test_interrupt_endpoint(handle, 0x83, "INT1");
    test_interrupt_endpoint(handle, 0x84, "INT2");

    // Try reading without sending first
    printf("\n=== Trying Spontaneous Reads ===\n");
    unsigned char buffer[8192];
    int transferred;

    printf("Trying to read from bulk endpoint 0x82...\n");
    ret = libusb_bulk_transfer(handle, 0x82, buffer, sizeof(buffer), &transferred, 1000);
    if (ret == 0 && transferred > 0) {
        printf("Received %d bytes spontaneously!\n", transferred);
        print_hex("Data", buffer, transferred > 64 ? 64 : transferred);
    } else {
        printf("No spontaneous data (expected): %s\n", libusb_error_name(ret));
    }

    // Cleanup
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);

    printf("\n=== Analysis Complete ===\n");
    printf("Check output above for any successful transfers.\n");
    printf("Next steps:\n");
    printf("- Review any vendor requests that returned data\n");
    printf("- Check if interrupt endpoints provide data\n");
    printf("- Capture this session with Wireshark for detailed analysis\n");

    return 0;
}

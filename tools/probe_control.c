/*
 * Control Transfer Protocol Explorer
 *
 * For Realtek/Microctopus MoC (USB ID 2541:fa03)
 *
 * This tool systematically explores vendor control transfers
 * that we discovered work with the device.
 *
 * Build: gcc probe_control.c -o probe_control -lusb-1.0
 * Run: sudo ./probe_control
 */

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define VID 0x2541
#define PID 0xfa03

void print_hex(const char *label, const unsigned char *data, int len) {
    printf("%s (%d bytes): ", label, len);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int vendor_read(libusb_device_handle *handle, uint8_t request, uint16_t value, uint16_t index,
                unsigned char *data, uint16_t length, const char *description) {
    printf("\n--- Vendor Read: %s ---\n", description);
    printf("Request: 0x%02X, Value: 0x%04X, Index: 0x%04X, Length: %d\n",
           request, value, index, length);

    int ret = libusb_control_transfer(handle,
                                      LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      request, value, index, data, length, 1000);

    if (ret < 0) {
        printf("ERROR: %s (%d)\n", libusb_error_name(ret), ret);
        return ret;
    }

    printf("SUCCESS: Received %d bytes\n", ret);
    if (ret > 0) {
        print_hex("Data", data, ret);
    }
    return ret;
}

int vendor_write(libusb_device_handle *handle, uint8_t request, uint16_t value, uint16_t index,
                 unsigned char *data, uint16_t length, const char *description) {
    printf("\n--- Vendor Write: %s ---\n", description);
    printf("Request: 0x%02X, Value: 0x%04X, Index: 0x%04X, Length: %d\n",
           request, value, index, length);
    if (length > 0) {
        print_hex("Sending", data, length);
    }

    int ret = libusb_control_transfer(handle,
                                      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                      request, value, index, data, length, 1000);

    if (ret < 0) {
        printf("ERROR: %s (%d)\n", libusb_error_name(ret), ret);
        return ret;
    }

    printf("SUCCESS: Sent %d bytes\n", ret);
    return ret;
}

int bulk_read(libusb_device_handle *handle, uint8_t endpoint, unsigned char *data,
              int length, const char *description) {
    printf("\n--- Bulk Read from 0x%02X: %s ---\n", endpoint, description);
    int transferred;
    int ret = libusb_bulk_transfer(handle, endpoint, data, length, &transferred, 1000);

    if (ret < 0) {
        printf("ERROR: %s (%d)\n", libusb_error_name(ret), ret);
        return ret;
    }

    printf("SUCCESS: Received %d bytes\n", transferred);
    if (transferred > 0) {
        print_hex("Data", data, transferred);
    }
    return transferred;
}

int main(void) {
    libusb_context *ctx = NULL;
    libusb_device_handle *handle = NULL;
    unsigned char buffer[512];
    int ret;

    printf("Control Transfer Protocol Explorer for Realtek 2541:fa03\n");
    printf("=======================================================\n\n");

    // Initialize
    libusb_init(&ctx);
    handle = libusb_open_device_with_vid_pid(ctx, VID, PID);
    if (!handle) {
        fprintf(stderr, "Failed to open device\n");
        libusb_exit(ctx);
        return 1;
    }

    // Detach kernel driver and claim interface
    if (libusb_kernel_driver_active(handle, 0) == 1) {
        libusb_detach_kernel_driver(handle, 0);
    }
    libusb_claim_interface(handle, 0);

    printf("Device opened and interface claimed\n");

    // Test the working vendor requests in detail
    printf("\n=== PHASE 1: Known Working Requests ===\n");

    memset(buffer, 0, sizeof(buffer));
    vendor_read(handle, 0x06, 0x0000, 0x0000, buffer, 64, "Request 0x06 (Status?)");

    memset(buffer, 0, sizeof(buffer));
    vendor_read(handle, 0x07, 0x0000, 0x0000, buffer, 64, "Request 0x07 (Device Info?)");

    // Read spontaneous data
    memset(buffer, 0, sizeof(buffer));
    bulk_read(handle, 0x82, buffer, 512, "Spontaneous data check");

    // Try variations of working requests
    printf("\n=== PHASE 2: Exploring Request 0x06 Variations ===\n");
    for (int val = 0; val < 4; val++) {
        memset(buffer, 0, sizeof(buffer));
        char desc[64];
        snprintf(desc, sizeof(desc), "Request 0x06 with value 0x%04X", val);
        vendor_read(handle, 0x06, val, 0, buffer, 64, desc);
        usleep(100000);
    }

    printf("\n=== PHASE 3: Exploring Request 0x07 Variations ===\n");
    for (int val = 0; val < 4; val++) {
        memset(buffer, 0, sizeof(buffer));
        char desc[64];
        snprintf(desc, sizeof(desc), "Request 0x07 with value 0x%04X", val);
        vendor_read(handle, 0x07, val, 0, buffer, 64, desc);
        usleep(100000);
    }

    // Try vendor writes to see if we can send commands
    printf("\n=== PHASE 4: Testing Vendor Writes ===\n");

    // Try simple vendor write with request 0x01 (common init command)
    unsigned char init_data[] = {0x01, 0x00, 0x00, 0x00};
    vendor_write(handle, 0x01, 0x0000, 0x0000, init_data, sizeof(init_data), "Init attempt");

    // Check if device responds
    memset(buffer, 0, sizeof(buffer));
    bulk_read(handle, 0x82, buffer, 512, "Response after init");

    // Try vendor write with request 0x02
    unsigned char reset_data[] = {0x02, 0x00, 0x00, 0x00};
    vendor_write(handle, 0x02, 0x0000, 0x0000, reset_data, sizeof(reset_data), "Reset attempt");

    memset(buffer, 0, sizeof(buffer));
    bulk_read(handle, 0x82, buffer, 512, "Response after reset");

    // Try writing to request 0x06 (since read worked)
    unsigned char cmd_data[] = {0x01};
    vendor_write(handle, 0x06, 0x0001, 0x0000, cmd_data, sizeof(cmd_data), "Write to 0x06");

    memset(buffer, 0, sizeof(buffer));
    bulk_read(handle, 0x82, buffer, 512, "Response after write to 0x06");

    // Explore more vendor request numbers
    printf("\n=== PHASE 5: Extended Vendor Request Scan ===\n");
    for (int req = 0x10; req < 0x20; req++) {
        memset(buffer, 0, sizeof(buffer));
        char desc[64];
        snprintf(desc, sizeof(desc), "Request 0x%02X", req);
        ret = vendor_read(handle, req, 0, 0, buffer, 64, desc);
        if (ret > 0) {
            printf("*** FOUND ANOTHER WORKING REQUEST! ***\n");
        }
        usleep(100000);
    }

    // Final status check
    printf("\n=== PHASE 6: Final Status ===\n");
    memset(buffer, 0, sizeof(buffer));
    vendor_read(handle, 0x06, 0x0000, 0x0000, buffer, 64, "Final status check");

    memset(buffer, 0, sizeof(buffer));
    vendor_read(handle, 0x07, 0x0000, 0x0000, buffer, 64, "Final device info");

    memset(buffer, 0, sizeof(buffer));
    bulk_read(handle, 0x82, buffer, 512, "Final bulk read");

    // Cleanup
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);

    printf("\n=== Exploration Complete ===\n");
    printf("Review the output above for patterns.\n");
    printf("Pay special attention to:\n");
    printf("- Request 0x06 and 0x07 responses\n");
    printf("- Bulk endpoint 0x82 spontaneous data\n");
    printf("- Any vendor writes that succeed\n");

    return 0;
}

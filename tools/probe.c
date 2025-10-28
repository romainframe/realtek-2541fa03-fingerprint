/*
 * USB Fingerprint Sensor Probe Tool
 *
 * For Realtek/Microctopus MoC (USB ID 2541:fa03)
 *
 * This tool attempts to communicate with the fingerprint sensor using
 * various USB protocols to reverse engineer the communication pattern.
 *
 * Based on CS9711 driver protocol as reference.
 *
 * Build: gcc probe.c -o probe -lusb-1.0
 * Run: sudo ./probe
 */

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define VID 0x2541
#define PID 0xfa03
#define EP_OUT 0x01
#define EP_IN_BULK 0x82
#define EP_IN_INT1 0x83
#define EP_IN_INT2 0x84

#define TIMEOUT_MS 1000

// CS9711 command structure
typedef struct {
    const char *name;
    uint8_t cmd[8];
} test_command_t;

// Test commands to try
test_command_t test_commands[] = {
    {"CS9711 Init",  {0xEA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0xEA}},
    {"CS9711 Reset", {0xEA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0xEA}},
    {"CS9711 Scan",  {0xEA, 0x04, 0x00, 0x00, 0x00, 0x00, 0x04, 0xEA}},
    {"Probe 0x00",   {0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA}},
    {"Probe 0x03",   {0xEA, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEA}},
    {"Probe 0x05",   {0xEA, 0x05, 0x00, 0x00, 0x00, 0x00, 0x05, 0xEA}},
    {"Short Init",   {0xEA, 0x01, 0xEA}},
    {"Alt Frame",    {0xEB, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0xEB}},
};

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

void print_error(const char *context, int error_code) {
    printf("ERROR in %s: %s (%d)\n", context, libusb_error_name(error_code), error_code);
}

int try_send_receive(libusb_device_handle *handle, uint8_t *cmd, int cmd_len,
                     uint8_t ep_in, const char *test_name) {
    int transferred = 0;
    int ret;

    printf("\n=== Test: %s ===\n", test_name);
    print_hex("Sending", cmd, cmd_len);

    // Send command
    ret = libusb_bulk_transfer(handle, EP_OUT, cmd, cmd_len, &transferred, TIMEOUT_MS);
    if (ret != 0) {
        print_error("Send", ret);
        return -1;
    }
    printf("Sent %d bytes successfully\n", transferred);

    // Try to receive response
    unsigned char response[8192];
    memset(response, 0, sizeof(response));

    ret = libusb_bulk_transfer(handle, ep_in, response, sizeof(response), &transferred, TIMEOUT_MS);
    if (ret != 0) {
        print_error("Receive", ret);
        return -1;
    }

    printf("Received %d bytes\n", transferred);
    if (transferred > 0) {
        print_hex("Response", response, transferred > 64 ? 64 : transferred);
        if (transferred > 64) {
            printf("... (%d more bytes)\n", transferred - 64);
        }
        return 0; // Success!
    }

    return -1;
}

int main(int argc, char *argv[]) {
    libusb_context *ctx = NULL;
    libusb_device_handle *handle = NULL;
    int ret;

    printf("Realtek 2541:fa03 Fingerprint Sensor Probe Tool\n");
    printf("================================================\n\n");

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
        fprintf(stderr, "Make sure device is connected and you have permissions (try sudo)\n");
        libusb_exit(ctx);
        return 1;
    }

    printf("Device %04X:%04X opened successfully\n", VID, PID);

    // Detach kernel driver if active
    if (libusb_kernel_driver_active(handle, 0) == 1) {
        printf("Kernel driver active, detaching...\n");
        ret = libusb_detach_kernel_driver(handle, 0);
        if (ret != 0) {
            fprintf(stderr, "Failed to detach kernel driver\n");
        }
    }

    // Claim interface
    ret = libusb_claim_interface(handle, 0);
    if (ret < 0) {
        fprintf(stderr, "Failed to claim interface: %s\n", libusb_error_name(ret));
        libusb_close(handle);
        libusb_exit(ctx);
        return 1;
    }

    printf("Interface 0 claimed successfully\n");
    printf("\nEndpoints: OUT=0x%02X, IN_BULK=0x%02X, IN_INT1=0x%02X, IN_INT2=0x%02X\n",
           EP_OUT, EP_IN_BULK, EP_IN_INT1, EP_IN_INT2);

    // Try each test command
    int num_tests = sizeof(test_commands) / sizeof(test_commands[0]);
    int successes = 0;

    for (int i = 0; i < num_tests; i++) {
        int cmd_len = 8;

        // Special handling for short commands
        if (strcmp(test_commands[i].name, "Short Init") == 0) {
            cmd_len = 3;
        }

        if (try_send_receive(handle, test_commands[i].cmd, cmd_len,
                            EP_IN_BULK, test_commands[i].name) == 0) {
            successes++;
            printf("✓ SUCCESS!\n");
        }

        usleep(500000); // Wait 500ms between tests
    }

    printf("\n=== Summary ===\n");
    printf("Total tests: %d\n", num_tests);
    printf("Successful: %d\n", successes);
    printf("Failed: %d\n", num_tests - successes);

    if (successes > 0) {
        printf("\n✓ At least one command got a response! Check above for details.\n");
    } else {
        printf("\n✗ No commands got a response. Device may use different protocol.\n");
        printf("  Next steps:\n");
        printf("  - Try interrupt endpoints with -i flag\n");
        printf("  - Capture USB traffic with Wireshark\n");
        printf("  - Check if device needs firmware upload\n");
    }

    // Cleanup
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(ctx);

    return successes > 0 ? 0 : 1;
}

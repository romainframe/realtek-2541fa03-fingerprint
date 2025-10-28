# USB Traffic Captures

This directory is for storing USB packet captures (.pcap files) from Wireshark/usbmon.

## Capturing USB Traffic

### Setup
```bash
# Load usbmon module
sudo modprobe usbmon

# Give user access to usbmon
sudo chmod a+r /sys/kernel/debug/usb/usbmon/*
```

### Using Wireshark
```bash
# Start Wireshark
sudo wireshark

# Select usbmon1 interface
# Filter: usb.bus_id == 1 && usb.device_address == 2
```

### Using tcpdump
```bash
# Capture to file
sudo tcpdump -i usbmon1 -w capture_$(date +%Y%m%d_%H%M%S).pcap

# In another terminal, run your test
cd ../tools
sudo ./probe
```

## File Naming Convention

Use descriptive names:
- `YYYYMMDD_HHMMSS_test_description.pcap`
- Example: `20251028_223000_cs9711_init_test.pcap`

## Important Note

.pcap files are excluded from git (see .gitignore) as they can be large.
Upload significant captures to GitHub releases or external storage if needed for sharing.

## Analysis

To analyze captures:
```bash
# View with Wireshark
wireshark capture_file.pcap

# Or use tshark for command line
tshark -r capture_file.pcap -Y "usb.src == 1.2.1"
```

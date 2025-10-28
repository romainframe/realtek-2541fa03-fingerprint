# Realtek 2541:fa03 Fingerprint Driver Development

Linux driver development project for the Realtek/Microctopus MoC fingerprint sensor (USB ID 2541:fa03) found in the Minisforum AI X1 Pro.

## Current Status

🔴 **Not Working** - Device detected by libfprint but communication fails with I/O errors.

## Device Information

- **USB Vendor ID**: `2541` (Chipsailing Electronics)
- **USB Product ID**: `fa03`
- **Device Name**: Realtek USB2.0 Finger Print Bridge / Microctopus MoC Fingerprint
- **Found In**: Minisforum AI X1 Pro mini PC
- **Interface Class**: 255 (Vendor Specific)

### USB Endpoints

| Endpoint | Direction | Type      | Max Packet Size |
|----------|-----------|-----------|-----------------|
| 0x01     | OUT       | Bulk      | 512 bytes       |
| 0x82     | IN        | Bulk      | 512 bytes       |
| 0x83     | IN        | Interrupt | 16 bytes        |
| 0x84     | IN        | Interrupt | 16 bytes        |

## Problem Statement

The device is not supported by any existing libfprint driver. An attempt to use it with the CS9711 driver (which supports similar Chipsailing devices 2541:0236 and 2541:9711) results in:

- Device is detected and opens successfully
- Communication timeout on initialization command
- USB I/O errors when attempting to read response

**Key Difference**: CS9711 uses endpoint `0x81` (IN) while this device uses `0x82` (IN).

## Project Goals

1. **Reverse engineer the USB protocol** for device communication
2. **Develop a working libfprint driver** or patch existing driver
3. **Enable fingerprint enrollment and authentication** on Linux
4. **Document the process** to help others with similar hardware

## Approach

### Phase 1: USB Protocol Analysis
- Use `usbmon` and Wireshark to capture USB traffic
- Create simple libusb test programs to probe the device
- Try variations of CS9711 protocol commands
- Test different endpoints and command structures

### Phase 2: Driver Development
- Modify CS9711 driver as starting point
- Implement discovered protocol
- Add extensive debug logging
- Iterative testing with fprintd

### Phase 3: Testing & Release
- Test enrollment and verification
- Handle error cases
- Create AUR package
- Submit upstream to libfprint

## Repository Structure

```
.
├── README.md              # This file
├── docs/
│   ├── device-info.md     # Detailed USB device information
│   ├── protocol-analysis.md    # Protocol reverse engineering notes
│   └── development-log.md # Detailed progress log
├── tools/
│   ├── probe.c            # Simple USB probe/test program
│   ├── Makefile           # Build tools
│   └── wireshark-filters.txt   # Useful Wireshark filters
├── driver/
│   ├── patches/           # Patches against libfprint
│   └── realtek_moc/       # Custom driver source
└── captures/              # USB traffic captures (.pcap)
```

## Related Hardware

### Similar Devices with Linux Support

| Device | USB ID | Status | Driver |
|--------|--------|--------|--------|
| Chipsailing CS9711 (dongle) | 2541:0236 | ✅ Working | libfprint-cs9711 |
| Chipsailing CS9711 (GPD) | 2541:9711 | ✅ Working | libfprint-cs9711 |
| FocalTech FT9366 | 2808:a658 | ✅ Working | libfprint-ft9366 |
| **Realtek/Microctopus MoC** | **2541:fa03** | ❌ **This Device** | **In Development** |

## Prerequisites

### Development Tools
```bash
sudo pacman -S base-devel git
sudo pacman -S libusb meson cmake
sudo pacman -S glib2-devel gobject-introspection
sudo pacman -S opencv doctest gtk-doc
sudo pacman -S wireshark-qt
```

### Fingerprint Software
```bash
sudo pacman -S fprintd libfprint imagemagick
```

## Quick Start

### 1. Clone Repository
```bash
git clone https://github.com/romainframe/realtek-2541fa03-fingerprint
cd realtek-2541fa03-fingerprint
```

### 2. Build Probe Tool
```bash
cd tools
make
```

### 3. Run Initial Tests
```bash
sudo ./probe
```

### 4. Check USB Traffic
```bash
sudo modprobe usbmon
sudo wireshark
# Filter: usb.bus_id == 1 && usb.device_address == 2
```

## Current Findings

### What Works
- ✅ Device is detected by Linux kernel
- ✅ Device is claimed by fprintd with modified CS9711 driver
- ✅ Device opens successfully without errors

### What Doesn't Work
- ❌ USB communication times out on init command
- ❌ Reading from endpoint 0x82 results in I/O errors
- ❌ Unknown if device requires different protocol or firmware

### Hypotheses
1. **Different Protocol**: Device may use different command structure than CS9711
2. **Firmware Upload**: Device may require firmware to be uploaded before use
3. **Match-on-Chip**: Device may use encrypted/proprietary protocol
4. **Endpoint Issue**: May need to use interrupt endpoints (0x83/0x84) instead

## Resources

### Documentation
- [libfprint Driver Development](https://fprint.freedesktop.org/libfprint-dev/)
- [CS9711 Driver Source](https://github.com/ddlsmurf/libfprint-CS9711)
- [USB Protocol Analysis Blog](https://infinytum.co/fixing-my-fingerprint-reader-on-linux-by-writing-a-driver-for-it/)

### Community
- [libfprint GitLab](https://gitlab.freedesktop.org/libfprint/libfprint)
- [Arch Linux Forums](https://bbs.archlinux.org/)
- [Framework Community](https://community.frame.work/) (active fingerprint discussions)

## Contributing

This is an experimental reverse engineering project. Contributions welcome:

- Test the probe tool on your device (if you have the same hardware)
- Capture USB traffic if you have Windows driver access
- Try protocol variations and document results
- Submit pull requests with findings

## License

LGPL-2.1-or-later (to match libfprint licensing)

## Acknowledgments

- CS9711 driver by ddlsmurf and contributors
- libfprint project and maintainers
- Arch Linux and AUR community

## Contact

- GitHub Issues: For bug reports and technical discussion
- GitHub Discussions: For questions and general help

---

**Note**: This is an active development project. The driver is not yet functional. Check the [development log](docs/development-log.md) for current progress and findings.

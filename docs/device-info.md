# Device Information: Realtek 2541:fa03

## USB Device Details

### Basic Information
- **Vendor ID**: `0x2541` (Chipsailing Electronics ShenZhen Co., Ltd)
- **Product ID**: `0xfa03`
- **Device Name**: Realtek USB2.0 Finger Print Bridge
- **Alternative Name**: Microctopus MoC Fingerprint
- **Manufacturer String**: Generic
- **Product String**: Realtek USB2.0 Finger Print Bridge

### Found In Hardware
- **Minisforum AI X1 Pro** (mini PC with AMD Ryzen AI 9 HX 370)
- Possibly other Minisforum devices with fingerprint sensors

## USB Configuration

### Interface Details
- **bInterfaceClass**: 255 (Vendor Specific)
- **bInterfaceSubClass**: 0
- **bInterfaceProtocol**: 0
- **Interface String**: Not specified

### Endpoints

| Address | Direction | Type      | Max Packet Size | Attributes |
|---------|-----------|-----------|-----------------|------------|
| 0x01    | OUT       | Bulk      | 512 bytes       | -          |
| 0x82    | IN        | Bulk      | 512 bytes       | -          |
| 0x83    | IN        | Interrupt | 16 bytes        | Interval: 1ms |
| 0x84    | IN        | Interrupt | 16 bytes        | Interval: 1ms |

### USB Device Path
```
Bus 001 Device 002: ID 2541:fa03
├─ Configuration 1
   └─ Interface 0
      ├─ Endpoint 0x01 OUT (Bulk, 512 bytes)
      ├─ Endpoint 0x82 IN (Bulk, 512 bytes)
      ├─ Endpoint 0x83 IN (Interrupt, 16 bytes, 1ms)
      └─ Endpoint 0x84 IN (Interrupt, 16 bytes, 1ms)
```

## Comparison with Similar Devices

### Chipsailing CS9711 (2541:0236)
**Similarities:**
- Same vendor ID (2541)
- Vendor specific class (255)
- Bulk OUT endpoint at 0x01
- Similar fingerprint bridge architecture

**Differences:**
- CS9711 uses endpoint 0x81 (IN), this device uses 0x82
- CS9711 product ID: 0x0236, this device: 0xfa03
- CS9711 works with existing driver, this device does not

### Chipsailing CS9711 GPD (2541:9711)
**Similarities:**
- Same as above

**Differences:**
- Different product ID (0x9711 vs 0xfa03)
- Found in GPD Win Max 2 2023 vs Minisforum AI X1 Pro

### FocalTech FT9366 (2808:a658)
**Similarities:**
- Match-on-Chip (MoC) fingerprint sensor
- Also marketed as "Realtek bridge"
- Proprietary protocol

**Differences:**
- Different vendor ID entirely (2808 vs 2541)
- Uses pre-compiled binary driver

## Technical Specifications

### Physical Sensor (Estimated)
Based on CS9711 driver, likely:
- **Sensor Width**: ~34-68 pixels
- **Sensor Height**: ~118-236 pixels
- **Scan Type**: Press (capacitive)
- **Technology**: Match-on-Chip (MoC)

### Match-on-Chip (MoC) Characteristics
MoC devices:
- Perform matching on the sensor chip itself
- More secure than sending raw fingerprint data
- May require proprietary firmware
- Often use encrypted communication
- May need cryptographic handshake

## Driver Detection Log

### Linux Kernel Detection
```
[   14.142358] Bluetooth: Core ver 2.22
usb 1-1: new full-speed USB device number 2 using xhci_hcd
usb 1-1: New USB device found, idVendor=2541, idProduct=fa03, bcdDevice= 0.00
usb 1-1: New USB device strings: Mfr=1, Product=2, SerialNumber=0
usb 1-1: Product: Realtek USB2.0 Finger Print Bridge
usb 1-1: Manufacturer: Generic
```

### fprintd Detection (with modified CS9711 driver)
```
fprintd: Device Chipsailing CS9711Fingprint scan type changed to 'press'
fprintd: Device Chipsailing CS9711Fingprint enroll stages changed to 16
fprintd: Finger present 0
fprintd: Finger needed 0
```

### Communication Failure
```
fprintd: Error while sending command 0x1, continuing anyway: transfer timed out
fprintd: libusb: error [submit_bulk_transfer] submiturb failed, errno=2
fprintd: Read failed: USB error on device 2541:fa03 : Input/Output Error [-1], aborting
```

## Protocol Observations

### From CS9711 Driver Attempt
1. Device opens successfully (interface claim works)
2. Init command (0x01) times out
3. Read from endpoint 0x82 results in I/O error
4. Device doesn't respond to CS9711 command structure

### Hypotheses
1. **Different Command Structure**: May not use 0xEA framing
2. **Firmware Upload Required**: May need firmware before accepting commands
3. **Different Endpoint Usage**: May require interrupt endpoints (0x83/0x84)
4. **MoC Protocol**: May use cryptographic handshake
5. **Different Vendor Protocol**: Despite same VID, may be completely different

## Windows Driver Information

### Windows Support
- Windows 10/11 drivers available from Minisforum
- Uses Windows Hello integration
- Driver likely proprietary from Realtek/Microctopus
- No public source code available

### Potential Analysis Methods
- USB packet capture on Windows (requires Windows installation)
- Driver binary analysis (advanced)
- Contact manufacturer for protocol documentation (unlikely to succeed)

## Next Steps for Analysis

1. **Try probe tool** with various command structures
2. **Capture USB traffic** if Windows available
3. **Test interrupt endpoints** (0x83, 0x84)
4. **Check for firmware upload** requirements
5. **Compare with other MoC devices** in libfprint

## References

- [USB.org Device Database](http://www.linux-usb.org/usb.ids)
- [Chipsailing Electronics](http://www.chipsailing.com/)
- [libfprint Supported Devices](https://fprint.freedesktop.org/supported-devices.html)
- [CS9711 Driver Source](https://github.com/ddlsmurf/libfprint-CS9711)

---

**Last Updated**: 2025-10-28
**Device Owner**: Minisforum AI X1 Pro with Arch Linux

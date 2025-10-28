# Protocol Analysis: Realtek 2541:fa03

## Discovery Date: 2025-10-28

## Summary

The Realtek/Microctopus MoC fingerprint sensor (2541:fa03) uses **vendor-specific control transfers** for communication, NOT bulk transfers like the CS9711 driver assumed. This is a fundamentally different protocol.

## Working Vendor Control Requests

### Request 0x06: Status/Device Info
```
Type: Vendor Read (IN)
bmRequestType: 0xC0 (LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE)
bRequest: 0x06
```

**Behavior with different wValue:**

| wValue | Response (Hex) | Length | Interpretation |
|--------|---------------|--------|----------------|
| 0x0000 | `DA 0B 13 58 00 00 32 00` | 8 bytes | Device ID/Version |
| 0x0001 | `00 00 00 00` | 4 bytes | Status (all clear) |
| 0x0002 | `00 00 00 00` | 4 bytes | Status (all clear) |
| 0x0003 | `00 00 00 00` | 4 bytes | Status (all clear) |

**Device ID Breakdown (DA 0B 13 58 00 00 32 00):**
- `DA 0B` = 0x0BDA = 3034 (possibly vendor-specific code)
- `13 58` = 0x5813 = 22547 (device model/revision?)
- `00 00` = Reserved/flags
- `32 00` = 0x0032 = 50 (decimal) - possibly firmware version?

### Request 0x07: Device Info (Duplicate)
```
Type: Vendor Read (IN)
bmRequestType: 0xC0
bRequest: 0x07
```

**Behavior:**
- With wValue 0x0000: Returns same data as Request 0x06 with wValue 0x0000
- With wValue 0x0001-0x0003: Returns 8 bytes of zeros
- Sometimes times out (inconsistent)

**Note:** This appears to be an alternate way to access the same info as 0x06.

### Request 0x15: Device Configuration/Properties
```
Type: Vendor Read (IN)
bmRequestType: 0xC0
bRequest: 0x15
wValue: 0x0000
wIndex: 0x0000
Length: 64 bytes
```

**Response (64 bytes):**
```
0A 00 00 00 00 00 03 06 B0 01 32 00 04 00 04 00
24 00 53 00 79 00 73 00 74 00 65 00 6D 00 57 00
61 00 6B 00 65 00 45 00 6E 00 61 00 62 00 6C 00
65 00 64 00 00 00 04 00 01 00 00 00 32 00 04 00
```

**Decoded:**
- Bytes 16-45 (UTF-16 LE): **"SystemWakeEnabled"**
- This is a Windows device property string
- Indicates device supports Windows property queries via vendor requests

**Structure Hypothesis:**
```
Offset  | Length | Content
--------|--------|--------
0x00    | 4      | Header: 0A 00 00 00
0x04    | 4      | Flags?: 00 00 03 06
0x08    | 4      | Value?: B0 01 32 00
0x0C    | 4      | Length indicators: 04 00 04 00
0x10    | 36     | UTF-16 string: "SystemWakeEnabled"
0x34    | 16     | Footer/additional data
```

## Bulk Endpoint Behavior

### Endpoint 0x82 (IN, Bulk)
- **Spontaneously sends data** without any command
- **Consistent response:** `01 F7 FF FF FF` (5 bytes)
- Does NOT timeout - data is always available

**Data Interpretation:**
```
Byte 0: 0x01 - Status/Message Type?
Byte 1-4: 0xF7 0xFF 0xFF 0xFF - Status flags or error code?
```

Possible meanings:
- `01` = "Ready" or "Idle" status
- `F7 FF FF FF` = All status bits set (possibly "no finger detected")

### Endpoint 0x01 (OUT, Bulk)
- All bulk write attempts **timeout**
- Device does NOT accept bulk OUT transfers
- This confirms device uses control transfers exclusively for commands

## Failed Attempts

### Vendor Write Requests
All vendor write (OUT) requests fail with `LIBUSB_ERROR_PIPE (-9)`:
- Request 0x01 (Init attempt)
- Request 0x02 (Reset attempt)
- Request 0x06 (Write attempt)

**Conclusion:** Device may not support vendor write requests, or requires specific setup first.

### Interrupt Endpoints
- Endpoint 0x83 (IN, Interrupt): Timeout
- Endpoint 0x84 (IN, Interrupt): Timeout

Both interrupt endpoints return no data during testing.

## Protocol Hypothesis

### Device Communication Model

```
PC                          Device
│                              │
├─[Vendor Read 0x06]─────────>│  Get device info
│<────[DA 0B 13 58...]────────┤
│                              │
├─[Vendor Read 0x15]─────────>│  Get configuration
│<────[SystemWakeEnabled...]──┤
│                              │
├─[Bulk Read 0x82]───────────>│  Poll status
│<────[01 F7 FF FF FF]─────── ┤  Device status
│                              │
├─[Unknown vendor write?]────>│  Send command?
│<────[Response via 0x82?]────┤
│                              │
```

### Next Steps for Protocol Discovery

1. **Scan more vendor request numbers**
   - Try 0x20-0xFF for reads
   - Try 0x08-0x14 (gap between working requests)

2. **Test vendor writes with different parameters**
   - Try writing with wValue/wIndex parameters
   - Try 0-length writes

3. **Monitor endpoint 0x82 during operations**
   - Check if spontaneous data changes
   - Look for event notifications

4. **Try control transfers with interface/endpoint recipients**
   - Current tests use DEVICE recipient
   - Try INTERFACE (0x01) and ENDPOINT (0x02) recipients

5. **Windows USB trace** (if available)
   - Capture actual Windows driver communication
   - Compare with our findings

## Comparison with CS9711

| Feature | CS9711 | This Device (2541:fa03) |
|---------|--------|------------------------|
| **Command Method** | Bulk OUT (0x01) | Vendor Control Transfers |
| **Response Method** | Bulk IN (0x81) | Bulk IN (0x82) + Control Transfers |
| **Framing** | 0xEA start/end markers | No framing (control transfers) |
| **Init Command** | Bulk write 8 bytes | Unknown vendor request |
| **Status Check** | Unknown | Vendor request 0x06/0x07 |
| **Endpoint Usage** | 0x01 OUT, 0x81 IN | Control EP + 0x82 IN |

## Key Findings

1. ✅ **Device uses vendor control transfers** - Not bulk transfers
2. ✅ **Three working vendor requests found** - 0x06, 0x07, 0x15
3. ✅ **Device sends spontaneous status** - Via endpoint 0x82
4. ✅ **Windows device properties exposed** - "SystemWakeEnabled"
5. ❌ **Vendor writes fail** - May need specific initialization
6. ❌ **Bulk OUT rejected** - Device doesn't accept bulk commands

## Probability of Success

**Updated Assessment: 60-70%** (improved from 40-60%)

**Why more likely now:**
- We can communicate with the device via control transfers
- We can read device info and status
- Device is responsive (not encrypted/locked)
- Clear protocol structure emerging

**Remaining challenges:**
- Need to find command/init vendor request
- Need to trigger fingerprint scanning
- Need to receive fingerprint image data
- May need Windows USB trace for full protocol

## Next Implementation Steps

1. Create extended vendor request scanner (0x00-0xFF)
2. Test vendor writes with various parameters
3. Monitor endpoint 0x82 for data changes
4. Build libfprint driver prototype using control transfers
5. Implement discovered vendor requests in driver

---

**Last Updated:** 2025-10-28
**Status:** Active Protocol Reverse Engineering
**Confidence:** High (can communicate with device)

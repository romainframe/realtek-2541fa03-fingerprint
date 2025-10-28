# TODO: Driver Development Tasks

## Phase 1: USB Protocol Analysis (Current Phase)

### Immediate Tasks
- [ ] Run probe tool and capture output
  ```bash
  cd ~/fingerprint-driver-dev/tools
  make
  sudo ./probe | tee ../captures/probe_output_$(date +%Y%m%d_%H%M%S).txt
  ```
- [ ] Document probe results in development log
- [ ] Analyze which commands (if any) get responses
- [ ] Set up Wireshark for USB traffic capture
- [ ] Capture USB traffic while running probe tool
- [ ] Analyze captured packets for patterns

### Protocol Testing
- [ ] Test CS9711 commands with endpoint 0x82
- [ ] Test interrupt endpoints (0x83, 0x84)
- [ ] Try different command byte patterns
- [ ] Test different frame markers (not just 0xEA)
- [ ] Try shorter/longer command lengths
- [ ] Test with different timeouts

### Analysis Tasks
- [ ] Compare captured traffic with CS9711 expected patterns
- [ ] Look for any device responses (even error responses)
- [ ] Check for firmware upload requirements
- [ ] Analyze USB descriptor for hidden endpoints
- [ ] Search for protocol documentation online

## Phase 2: Driver Development (If Phase 1 Successful)

### Setup
- [ ] Clone libfprint repository
  ```bash
  cd ~/fingerprint-driver-dev/driver
  git clone https://gitlab.freedesktop.org/libfprint/libfprint.git
  ```
- [ ] Set up build environment for libfprint
- [ ] Create custom driver directory based on CS9711

### Implementation
- [ ] Create realtek_moc driver skeleton
- [ ] Add USB ID 2541:fa03 to driver
- [ ] Implement discovered protocol commands
- [ ] Add extensive debug logging
- [ ] Implement state machine for init/scan
- [ ] Handle fingerprint image capture
- [ ] Process image data if needed

### Testing
- [ ] Build driver with debug symbols
- [ ] Test device detection
- [ ] Test communication with fprintd
- [ ] Test fingerprint enrollment
- [ ] Test fingerprint verification
- [ ] Handle error cases

## Phase 3: Documentation & Release (If Phase 2 Successful)

### Documentation
- [ ] Document complete protocol in docs/protocol-analysis.md
- [ ] Update development log with final findings
- [ ] Write installation instructions
- [ ] Create troubleshooting guide
- [ ] Add code comments and documentation

### Packaging
- [ ] Create PKGBUILD for AUR
- [ ] Test installation on clean system
- [ ] Create GitHub release
- [ ] Submit patch to libfprint upstream

### Community
- [ ] Post to Arch Linux forums
- [ ] Create issue on libfprint GitLab
- [ ] Update README with success status
- [ ] Help others with same hardware

## Decision Points

### After Probe Testing (4 hours max)
**Question**: Does device respond to ANY commands?
- ✅ YES → Continue to protocol analysis
- ❌ NO → Try interrupt endpoints and alternative approaches
- ❌ Still NO → May need Windows traffic capture or accept limitation

### After Protocol Analysis (12 hours max)
**Question**: Can we identify the communication protocol?
- ✅ YES → Begin driver implementation
- ❌ NO → Document findings and seek community help
- ⚠️  PARTIAL → Continue iterating on protocol variations

### After Driver Development (20 hours total)
**Question**: Can we capture fingerprint images?
- ✅ YES → Continue to enrollment/verification
- ❌ NO → Re-analyze protocol or pause project
- ⚠️  PARTIAL → Debug and iterate

## Alternative Approaches (If Main Path Fails)

### Fallback Options
- [ ] Search for leaked Windows driver binaries to analyze
- [ ] Contact Minisforum support for protocol documentation
- [ ] Try to find other users with same device
- [ ] Set up Windows VM for USB traffic capture
- [ ] Use alternative authentication methods (password, facial recognition)

### Community Help
- [ ] Post detailed findings to libfprint GitLab
- [ ] Ask on Arch Linux forums
- [ ] Share captures and logs for analysis
- [ ] Collaborate with others working on similar sensors

## Current Status

**Phase**: Phase 1 - USB Protocol Analysis
**Started**: 2025-10-28
**Last Updated**: 2025-10-28

**Completed**:
- ✅ Repository setup
- ✅ Documentation structure
- ✅ Probe tool created
- ✅ Initial device analysis

**In Progress**:
- ⏳ Running probe tests
- ⏳ USB traffic capture

**Blocked**:
- None currently

## Time Tracking

| Phase | Estimated | Actual | Status |
|-------|-----------|--------|--------|
| Phase 1: Protocol Analysis | 6-8 hours | 2 hours | In Progress |
| Phase 2: Driver Development | 8-12 hours | - | Pending |
| Phase 3: Documentation | 4-6 hours | - | Pending |
| **Total** | **20-28 hours** | **2 hours** | **8% Complete** |

## Notes

- Focus on systematic testing and documentation
- Don't spend more than 4 hours on any single approach
- Document all failures (they're valuable data too)
- Commit progress regularly to GitHub
- Ask for help if stuck for more than 2 hours on same issue

## Quick Commands Reference

```bash
# Build probe tool
cd ~/fingerprint-driver-dev/tools && make

# Run probe with logging
sudo ./probe | tee ../captures/probe_$(date +%Y%m%d_%H%M%S).txt

# Start USB capture
sudo modprobe usbmon
sudo wireshark  # Select usbmon1, filter: usb.idVendor == 0x2541

# Check device status
lsusb | grep 2541
journalctl -u fprintd -n 50

# Git workflow
git add -A
git commit -m "Description of changes"
git push
```

---

**Legend**:
- [ ] Not started
- [x] Completed
- ⏳ In progress
- ⚠️ Blocked/Issues
- ❌ Failed/Abandoned

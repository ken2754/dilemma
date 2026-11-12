# Dilemma 3X5 (guitar_procyon35) with Procyon Touchpad

A split keyboard with 35 physical keys (3x5 column-staggered + thumb cluster), dual rotary encoders, RGB matrix, and a Microchip maXTouch Procyon touchpad, powered by Raspberry Pi RP2040 and running [Vial](https://get.vial.today/)-compatible QMK firmware.

---

## Hardware Specifications

- **Microcontroller**: Dual Raspberry Pi RP2040 (16MB Flash)
- **Split Handedness**: Hardware pin detection on `GP29` (`SPLIT_HAND_PIN_LOW_IS_RIGHT`: Low = Left master/slave, High = Right)
- **Split Communication**: Full-duplex hardware USART serial over `GP0` (TX) and `GP1` (RX)
- **Pointing Device**:
  - Sensor: Microchip maXTouch mXT144UD trackpad module (Procyon 42/50) located on the right half
  - Interface: Hardware I2C1 (`GP2` SDA, `GP3` SCL) running at 1 MHz (`I2C1_CLOCK_SPEED 1000000`)
  - Interrupt: Motion detection on `GP11` (Active Low)
- **Rotary Encoders**: Supported on both left and right halves
- **Backlight / Underglow**: Per-key WS2812 RGB Matrix driven via RP2040 PIO

---

## Features & Implementation Details

### 1. Touchpad Implementation & Tuning
- **Cursor Sensitivity (CPI)**: Configured to `TOUCHPAD_DEFAULT_CPI 800` (calm, predictable cursor motion without over-sensitivity).
- **Lift-off Noise Filter**: Modified driver filters out the release-bounce coordinates (`MXT_UP` event), preventing the cursor from jumping when lifting fingers.
- **Gesture Control**:
  - **1-Finger**: Smooth, high-precision cursor movement.
  - **2-Finger**: Vertical and horizontal scrolling.
  - **Taps / Dragging**: Tap-to-click gestures are **completely disabled** in firmware. Mouse clicks are handled exclusively by physical keyboard keys (Vial mouse keys). This design choice eliminates accidental clicks and 100% prevents host-side mouse button hold/stuck bugs on Debian/Linux and other operating systems.

### 2. RGB Matrix Default Behavior
- Configured to be **OFF by default** on power-up (`RGB_MATRIX_DEFAULT_ON false`, `val: 0`, and explicitly turned off in `keyboard_post_init_user()`) to prevent blinding breathing animations upon plugging in.
- Can be turned on, toggled, and customized anytime via Vial or QMK RGB keycodes.

### 3. Rotary Encoders
- Left and right encoder actions are fully customizable on every layer directly within Vial.

---

## Vial Configuration

The Vial keyboard definition is embedded directly into the firmware. Connecting the keyboard to [Vial Web](https://vial.rocks/) or the Vial desktop application will automatically recognize the layout without needing to load an external JSON file.

- **Vial Unlock Combo**: `KC_Q` and `KC_W` (Matrix positions `(0,0)` and `(0,1)`)
- **Security**: `VIAL_INSECURE` is enabled by default for immediate plug-and-play access.
- **Layout Definition**: If sideloading is required, use `vial.json` provided in the repository.

---

## Building the Firmware

### Prerequisites
Clone the [vial-qmk](https://github.com/vial-kb/vial-qmk) repository:

```bash
git clone --recurse-submodules https://github.com/vial-kb/vial-qmk.git
cd vial-qmk
```

### Setup & Compilation
Symlink or copy this keyboard folder into `vial-qmk/keyboards/dilemma35`:

```bash
ln -s /path/to/guitar_procyon35 keyboards/dilemma35
```

Build the Vial firmware:

```bash
make dilemma35:vial
```

Or using QMK CLI:

```bash
qmk compile -kb dilemma35 -km vial
```

The compiled UF2 file `dilemma35_vial.uf2` will be created in the `.build/` directory (and root of `vial-qmk`).

---

## Flashing the Firmware

Put the RP2040 controller into bootloader mode using any of the following methods:

1. **Double-Tap Reset**: Double-click the physical reset button on the PCB (`RP2040_BOOTLOADER_DOUBLE_TAP_RESET` enabled).
2. **Bootmagic Reset**: Hold down the top-left key (`KC_Q`, matrix `0,0`) while plugging in the USB cable.
3. **Hardware Reset**: Press and hold the `BOOT` button, press `RESET`, then release `BOOT`.

Once the `RPI-RP2` mass storage drive appears on your computer, drag and drop `dilemma35_vial.uf2` onto it. The controller will automatically flash and reboot.

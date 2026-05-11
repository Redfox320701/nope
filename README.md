# Loves Me — Flipper Zero App

A "loves me, loves me not" daisy petal game for the Flipper Zero.

## How it works

- A flower with 30 petals is drawn on screen
- Press **OK** (or right arrow) to pluck a petal
- Each petal alternates between **"Loves me!"** and **"Loves me not..."**
- The final outcome is **randomized** each game — so the flower always surprises you
- Press **Back** (hold) on the result screen to play again
- Press **Back** at any time to exit

## Controls

| Button | Action |
|--------|--------|
| OK / Right | Pluck a petal |
| Back (short) | Exit app |
| Back (hold) | Restart (on result screen) |

## Building

You need the [Flipper Zero firmware build environment](https://github.com/flipperdevices/flipperzero-firmware) set up.

### Steps

1. Clone the Flipper Zero firmware:
   ```
   git clone --recursive https://github.com/flipperdevices/flipperzero-firmware.git
   ```

2. Copy this folder into the `applications_user/` directory:
   ```
   cp -r loves_me flipperzero-firmware/applications_user/
   ```

3. Build the FAP:
   ```
   cd flipperzero-firmware
   ./fbt fap_loves_me
   ```

4. The compiled `.fap` file will be at:
   ```
   build/f7-firmware-D/.extapps/loves_me.fap
   ```

5. Copy `loves_me.fap` to your Flipper Zero's SD card under:
   ```
   /ext/apps/Games/loves_me.fap
   ```

6. On the Flipper, go to **Apps → Games → Loves Me** and run it!

## Icon

Place a 10x10 pixel black-and-white PNG named `icon.png` in this folder before building.
A simple flower or heart works well.

## Notes

- Tested against Flipper Zero firmware 0.95+
- Uses the standard GUI canvas API — no extra dependencies
- Stack size: 1 KB (lightweight)

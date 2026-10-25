#!/usr/bin/env python3
"""
PipBoy Rotary Encoder Submenu Controller

Controls the Pipboy (PIP-OS) submenu navigation using a rotary encoder and push button
connected to the Raspberry Pi GPIO pins.

Default Wiring (BCM GPIO):
  - Rotary Encoder Pin A (CLK): GPIO 17
  - Rotary Encoder Pin B (DT):  GPIO 27
  - Rotary Encoder Switch (SW): GPIO 22
  - VCC: 3.3V
  - GND: Ground

Key Mappings:
  - Rotate Clockwise:        SUB_TAB_NEXT     (Right Arrow)
  - Rotate Counter-Clockwise: SUB_TAB_PREVIOUS (Left Arrow)
  - Button Pressed:          BUTTON_SELECT    (Space / Enter)
"""

import argparse
import os
import sys
import time
from signal import pause

uinput_device = None
pynput_keyboard = None
backend_type = None

# Tab cycling order for optional cycle_tab action
MAIN_TABS = ["TAB_STAT", "TAB_ITEM", "TAB_DATA", "TAB_MAP", "TAB_RADIO"]
current_tab_idx = 0


def init_input_backend(verbose=False):
    """Initializes the virtual input backend (evdev, pynput, or dummy fallback)."""
    global uinput_device, pynput_keyboard, backend_type

    # 1. Try evdev (Linux /dev/uinput - ideal for Raspberry Pi & EGLFS/KMS)
    try:
        import evdev
        from evdev import UInput, ecodes as e

        capabilities = {
            e.EV_KEY: [
                e.KEY_RIGHT,  # SUB_TAB_NEXT
                e.KEY_LEFT,   # SUB_TAB_PREVIOUS
                e.KEY_SPACE,  # BUTTON_SELECT
                e.KEY_ENTER,
                e.KEY_UP,     # SCROLL_UP
                e.KEY_DOWN,   # SCROLL_DOWN
                e.KEY_F1,     # TAB_STAT
                e.KEY_F2,     # TAB_ITEM
                e.KEY_F3,     # TAB_DATA
                e.KEY_F4,     # TAB_MAP
                e.KEY_F5,     # TAB_RADIO
            ]
        }
        try:
            uinput_device = UInput(capabilities, name="PipBoy Rotary Controller")
            backend_type = "evdev"
            if verbose:
                print("[Input Backend] Using Linux uinput (evdev).")
            return True
        except PermissionError:
            print("Warning: Permission denied accessing /dev/uinput.")
            print("To fix, add your user to the input group:")
            print("    sudo usermod -aG input $USER")
            print("Or configure udev permissions:")
            print("    echo 'KERNEL==\"uinput\", GROUP=\"input\", MODE=\"0660\"' | sudo tee /etc/udev/rules.d/99-uinput.rules")
            print("    sudo udevadm control --reload-rules && sudo udevadm trigger")
            print("Attempting desktop fallback...")
    except ImportError:
        pass

    # 2. Try pynput (Desktop environments: X11 / Wayland / macOS)
    try:
        from pynput.keyboard import Controller
        pynput_keyboard = Controller()
        backend_type = "pynput"
        if verbose:
            print("[Input Backend] Using pynput.")
        return True
    except (ImportError, Exception):
        pass

    backend_type = "dummy"
    print("[Input Backend] Running in logging/dummy mode (no evdev or pynput available).")
    return True


def emit_key(action, verbose=False):
    """Emits the keyboard event for the given action."""
    global current_tab_idx

    if action == "CYCLE_TAB":
        action = MAIN_TABS[current_tab_idx]
        current_tab_idx = (current_tab_idx + 1) % len(MAIN_TABS)

    if verbose:
        print(f"[Key Event] {action}")

    if backend_type == "evdev" and uinput_device is not None:
        from evdev import ecodes as e
        key_map = {
            "SUB_TAB_NEXT": e.KEY_RIGHT,
            "SUB_TAB_PREVIOUS": e.KEY_LEFT,
            "BUTTON_SELECT": e.KEY_SPACE,
            "ENTER": e.KEY_ENTER,
            "SCROLL_UP": e.KEY_UP,
            "SCROLL_DOWN": e.KEY_DOWN,
            "TAB_STAT": e.KEY_F1,
            "TAB_ITEM": e.KEY_F2,
            "TAB_DATA": e.KEY_F3,
            "TAB_MAP": e.KEY_F4,
            "TAB_RADIO": e.KEY_F5,
        }
        code = key_map.get(action)
        if code is not None:
            uinput_device.write(e.EV_KEY, code, 1)
            uinput_device.write(e.EV_SYN, e.SYN_REPORT, 0)
            time.sleep(0.01)
            uinput_device.write(e.EV_KEY, code, 0)
            uinput_device.write(e.EV_SYN, e.SYN_REPORT, 0)

    elif backend_type == "pynput" and pynput_keyboard is not None:
        from pynput.keyboard import Key
        key_map = {
            "SUB_TAB_NEXT": Key.right,
            "SUB_TAB_PREVIOUS": Key.left,
            "BUTTON_SELECT": Key.space,
            "ENTER": Key.enter,
            "SCROLL_UP": Key.up,
            "SCROLL_DOWN": Key.down,
            "TAB_STAT": Key.f1,
            "TAB_ITEM": Key.f2,
            "TAB_DATA": Key.f3,
            "TAB_MAP": Key.f4,
            "TAB_RADIO": Key.f5,
        }
        val = key_map.get(action)
        if val is not None:
            pynput_keyboard.press(val)
            time.sleep(0.01)
            pynput_keyboard.release(val)


def main():
    parser = argparse.ArgumentParser(description="PipBoy Rotary Encoder Submenu Controller")
    parser.add_argument("--clk", type=int, default=17, help="BCM GPIO pin for Rotary Encoder CLK/A (default: 17)")
    parser.add_argument("--dt", type=int, default=27, help="BCM GPIO pin for Rotary Encoder DT/B (default: 27)")
    parser.add_argument("--sw", type=int, default=22, help="BCM GPIO pin for Push Button SW (default: 22)")
    parser.add_argument("--bounce-time", type=float, default=0.05, help="Button debounce time in seconds (default: 0.05)")
    parser.add_argument("--button-action", choices=["select", "enter", "cycle_tab", "next_sub", "prev_sub"],
                        default="select", help="Action when encoder button is pressed (default: select)")
    parser.add_argument("--invert", action="store_true", help="Invert rotation direction")
    parser.add_argument("--verbose", "-v", action="store_true", help="Enable verbose debug output")
    parser.add_argument("--simulate", action="store_true", help="Simulate encoder using keyboard commands (for testing without GPIO)")
    args = parser.parse_args()

    init_input_backend(verbose=args.verbose)

    button_action_map = {
        "select": "BUTTON_SELECT",
        "enter": "ENTER",
        "cycle_tab": "CYCLE_TAB",
        "next_sub": "SUB_TAB_NEXT",
        "prev_sub": "SUB_TAB_PREVIOUS",
    }
    target_button_action = button_action_map[args.button_action]

    if args.simulate:
        print("Starting simulated rotary controller.")
        print("Type '>' or '.' for Clockwise, '<' or ',' for Counter-Clockwise, 'b' or 'space' for Button press. 'q' to quit.")
        last_sim_steps = 0
        try:
            while True:
                cmd = input().strip()
                if cmd in (">", "."):
                    last_sim_steps += 1
                    print("Position:", last_sim_steps)
                    emit_key("SUB_TAB_PREVIOUS" if args.invert else "SUB_TAB_NEXT", verbose=args.verbose)
                elif cmd in ("<", ","):
                    last_sim_steps -= 1
                    print("Position:", last_sim_steps)
                    emit_key("SUB_TAB_NEXT" if args.invert else "SUB_TAB_PREVIOUS", verbose=args.verbose)
                elif cmd in ("b", " ", "btn"):
                    print("Button pressed!")
                    emit_key(target_button_action, verbose=args.verbose)
                elif cmd == "q":
                    break
        except (KeyboardInterrupt, EOFError):
            pass
        return

    try:
        from gpiozero import RotaryEncoder, Button
    except ImportError:
        print("Error: gpiozero is not installed.")
        print("Please install gpiozero with: sudo apt install python3-gpiozero")
        print("(Or run with --simulate to test without Raspberry Pi GPIO hardware)")
        sys.exit(1)

    # Rotary encoder: BCM GPIO 17 and GPIO 27
    encoder = RotaryEncoder(args.clk, args.dt, max_steps=0)

    # Push button: BCM GPIO 22
    button = Button(args.sw, pull_up=True, bounce_time=args.bounce_time)

    last_steps = 0

    def rotated():
        nonlocal last_steps
        current_steps = encoder.steps
        diff = current_steps - last_steps
        last_steps = current_steps

        print("Position:", encoder.steps)

        if diff > 0:
            for _ in range(diff):
                action = "SUB_TAB_PREVIOUS" if args.invert else "SUB_TAB_NEXT"
                emit_key(action, verbose=args.verbose)
        elif diff < 0:
            for _ in range(-diff):
                action = "SUB_TAB_NEXT" if args.invert else "SUB_TAB_PREVIOUS"
                emit_key(action, verbose=args.verbose)

    def pressed():
        print("Button pressed!")
        emit_key(target_button_action, verbose=args.verbose)

    encoder.when_rotated = rotated
    button.when_pressed = pressed

    print(f"PipBoy Rotary Encoder active on GPIO {args.clk}/{args.dt} (SW: GPIO {args.sw}).")
    print("Turn the encoder or press its button. Ctrl+C to exit.")
    try:
        pause()
    except KeyboardInterrupt:
        print("\nExiting PipBoy Rotary Encoder Controller.")


if __name__ == "__main__":
    main()

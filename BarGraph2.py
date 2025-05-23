# https://github.com/nickFridays/RaspberryPi-5__Python/blob/main/BarGraph2.py
# SN74HC595N shift register   https://www.ti.com/lit/ds/symlink/sn74hc595.pdf
# for Mikroe BarGraph-2       https://www.mikroe.com/bargraph-2-click

from machine import Pin, SPI
import utime
from utime import sleep
from utime import sleep_ms

class BarGraph2:
    LED_SEGMENTS = [0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]
    POWER_ON = 1
    POWER_OFF = 0

    def __init__(self, spi, cs_pin, mr_pin, pwm_pin):
        self.spi = spi
        self.cs = Pin(cs_pin, Pin.OUT,1)
        self.mr = Pin(mr_pin, Pin.OUT,0)
        self.pwm = Pin(pwm_pin, Pin.OUT,0)
        #self.cs.value(1)  # Deselect the device
        #self.pwm.value(0) # 0 Power off by default
        #self.mr.value(0)
    def reset(self):
        self.mr.value(0)
        sleep(0.1)
        self.mr.value(1)

    def write_byte(self, data):
        self.cs.value(0)  # Select the device
        self.spi.write(bytearray([data]))
        self.cs.value(1)  # Deselect the device

    def segment_switch(self, reg_3, reg_2, reg_1):
        self.reset()
        self.write_byte(reg_3)
        self.write_byte(reg_2)
        self.write_byte(reg_1)

    def lights_off(self):
        self.segment_switch(0x00, 0x00, 0x00)

    def power(self, on_off):
        self.pwm.value(on_off)
        
    def led_green(self, index):
        """Turn on LED at 'index' in green (1-based index: 1–10)."""
        reg_1, reg_2, reg_3 = 0, 0, 0
        if 1 <= index <= 8:
            reg_1 = 1 << (index - 1)
        elif index == 9:
            reg_2 = 1  # reg_2 bit 0
        elif index == 10:
            reg_2 = 2  # reg_2 bit 1
        self.segment_switch(reg_3, reg_2, reg_1)

    def led_red(self, index):
        """Turn on LED at 'index' in red, according to the provided truth table."""
        if 1 <= index <= 6:
            self.segment_switch(0, 2 ** (index + 1), 0)
        elif 7 <= index <= 10:
            self.segment_switch(2 ** (index - 7), 0, 0)
        else:
            self.lights_off()

    def led_yellow(self, index):
        """Turn on LED at 'index' in yellow, according to the provided truth table."""
        if 1 <= index <= 6:
            self.segment_switch(0, 2 ** (index + 1), 2 ** (index - 1))
        elif index == 7:
            self.segment_switch(1, 0, 64)       # reg_3=1, reg_1=64
        elif index == 8:
            self.segment_switch(2, 0, 128)      # reg_3=2, reg_1=128
        elif index == 9:
            self.segment_switch(4, 1, 0)        # reg_3=4, reg_2=1
        elif index == 10:
            self.segment_switch(8, 2, 0)        # reg_3=8, reg_2=2
        else:
            self.lights_off()

    def led_range(self, start=1, end=10, color='green', delay=0.2):
        """
        Turn on LEDs from 'start' to 'end' (inclusive) in sequence with given color and delay.
        color: 'green', 'red', or 'yellow'
        """
        for idx in range(start, end + 1):
            if color == 'green':
                self.led_green(idx)
            elif color == 'red':
                self.led_red(idx)
            elif color == 'yellow':
                self.led_yellow(idx)
            else:
                raise ValueError("Color must be 'green', 'red', or 'yellow'")
            sleep(delay)
        self.lights_off()  # optional: turn off after sequence


    def led_range_all(self, start=1, end=10, color='red', delay=0.2):
        """
        Turns on LEDs in the range [start, end] one by one, so that at the end all LEDs in the range are ON.
        The color can be 'red', 'green', or 'yellow'.
        """
        # Prepare accumulators for reg_3, reg_2, reg_1
        reg_1 = reg_2 = reg_3 = 0

        for idx in range(start, end + 1):
            if color == 'green':
                # Green mapping from your table
                if 1 <= idx <= 8:
                    reg_1 |= 1 << (idx - 1)
                elif idx == 9:
                    reg_2 |= 1
                elif idx == 10:
                    reg_2 |= 2
            elif color == 'red':
                if 1 <= idx <= 6:
                    reg_2 |= 2 ** (idx + 1)
                elif 7 <= idx <= 10:
                    reg_3 |= 2 ** (idx - 7)
            elif color == 'yellow':
                if 1 <= idx <= 6:
                    reg_2 |= 2 ** (idx + 1)
                    reg_1 |= 2 ** (idx - 1)
                elif idx == 7:
                    reg_3 |= 1
                    reg_1 |= 64
                elif idx == 8:
                    reg_3 |= 2
                    reg_1 |= 128
                elif idx == 9:
                    reg_3 |= 4
                    reg_2 |= 1
                elif idx == 10:
                    reg_3 |= 8
                    reg_2 |= 2
            else:
                raise ValueError("Color must be 'red', 'green', or 'yellow'")
            # Light up current state to show progressive accumulation
            self.segment_switch(reg_3, reg_2, reg_1)
            sleep(delay)
        # Final call ensures all are ON (if not already set in last loop)
        self.segment_switch(reg_3, reg_2, reg_1)


# Usage 
def main():
    # Blink LED to show the start
    led = machine.Pin(25, machine.Pin.OUT) # On-board LED
    for i in range(4):
        sleep(0.1)
        led.value(1)
        sleep(0.1)
        led.value(0)            
    # Inst BarGraph2 class
    spi = SPI(0, baudrate=100000, polarity=0, phase=0, sck=Pin(18), mosi=Pin(19), miso=Pin(16))
    bargraph = BarGraph2(spi, cs_pin=17, mr_pin=20, pwm_pin=2)
    bargraph.reset()
    sleep(.1)
    bargraph.power(BarGraph2.POWER_ON)
    sleep(.1)
    bargraph.reset()
    sleep(.1)
    bargraph.segment_switch(0, 0, 0)  # all  LEDs OFF   
    bargraph.led_range(start=1, end=10, color='red', delay=0.1)
    bargraph.led_range(start=1, end=10, color='yellow', delay=0.1)
    bargraph.led_range(start=1, end=10, color='green', delay=0.1)
    bargraph.led_range_all(start=1, end=10, color='red', delay=0.1)
    bargraph.led_range_all(start=1, end=10, color='yellow', delay=0.1)
    bargraph.led_range_all(start=1, end=10, color='green', delay=0.1)
     

if __name__ == "__main__":
    main()

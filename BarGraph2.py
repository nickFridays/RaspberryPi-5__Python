# MIKROE-3021 BarGraph-2   
# Micropython code for Raspberry Pi Pico 
# to write a list of bits to 3 daisy chained SN74HC595N shift registers driving 10 LEDs
# SN74HC595N shift register   https://www.ti.com/lit/ds/symlink/sn74hc595.pdf
# Mikroe BarGraph-2       https://www.mikroe.com/bargraph-2-click

from machine import Pin, SPI
import utime
from utime import sleep
from utime import sleep_ms

        
POWER_ON = 1
POWER_OFF = 0

class BarGraph2:
    def __init__(self, spi, cs_pin, mr_pin, pwm_pin):
        """
        Initialize BarGraph2 with SPI and pin definitions.
        Sets up the SN74HC595N shift register daisy chain and color lookup tables.
        """
        self.spi = spi
        self.cs = Pin(cs_pin, Pin.OUT, value=1)
        self.mr = Pin(mr_pin, Pin.OUT, value=0)
        self.pwm = Pin(pwm_pin, Pin.OUT, value=0)
        # Color lookup tables: each entry is [reg3, reg2, reg1] for that LED and color.
        self.Green = [[0,0,1],[0,0,2],[0,0,4],[0,0,8],[0,0,16],[0,0,32],[0,0,64],[0,0,128],[0,1,0],[0,2,0]]
        self.Red = [[0,4,0],[0,8,0],[0,16,0],[0,32,0],[0,64,0],[0,128,0],[1,0,0],[2,0,0],[4,0,0],[8,0,0]]
        self.Yellow = [[0,4,1],[0,8,2],[0,16,4],[0,32,8],[0,64,16],[0,128,32],[1,0,64],[2,0,128],[4,1,0],[8,2,0]]
        self.Graph=[self.Green, self.Yellow, self.Red]

    def reset(self):
        """
        Reset the shift registers by pulsing the MR (Master Reset) pin.
        """
        self.mr.value(0)
        sleep(0.02)
        self.mr.value(1)

    def write_byte(self, data):
        """
        Write a single byte to the daisy-chained shift registers via SPI.
        Args:
            data: 8-bit value to write to the registers.
        """
        self.cs.value(0)
        self.spi.write(bytearray([data]))
        self.cs.value(1)

    def segment_switch(self, reg_3, reg_2, reg_1):
        """
        Set the outputs of all three shift registers to drive the bargraph LEDs.
        Args:
            reg_3, reg_2, reg_1: Register values for the three SN74HC595Ns.
        """
        # self.reset()  # Optionally reset before every update
        self.write_byte(reg_3)
        self.write_byte(reg_2)
        self.write_byte(reg_1)

    def lights_off(self):
        """
        Turn off all LEDs.
        """
        self.segment_switch(0, 0, 0)

    def power(self, on_off):
        """
        Power on or off the bargraph circuit.
        Args:
            on_off: Set to 1 (on) or 0 (off).
        """
        self.pwm.value(on_off)

    def led_color(self, index, color_list):
        """
        Turn on a specific LED at a given color.
        Args:
            index: LED number (1-10).
            color_list: One of self.Green, self.Yellow, self.Red.
        """
        if 1 <= index <= 10:
            reg3, reg2, reg1 = color_list[index-1]
            self.segment_switch(reg3, reg2, reg1)
        else:
            self.lights_off()

    def led_green(self, index):
        """
        Turn on LED at 'index' in green.
        Args:
            index: LED number (1-10).
        """
        self.led_color(index, self.Green)

    def led_red(self, index):
        """
        Turn on LED at 'index' in red.
        Args:
            index: LED number (1-10).
        """
        self.led_color(index, self.Red)

    def led_yellow(self, index):
        """
        Turn on LED at 'index' in yellow.
        Args:
            index: LED number (1-10).
        """
        self.led_color(index, self.Yellow)

    def led_range(self, start=1, end=10, color='green', delay=0.2):
        """
        Animate LEDs lighting up in sequence from start to end (or in reverse).
        Args:
            start: Starting LED index.
            end: Ending LED index.
            color: 'green', 'red', or 'yellow'.
            delay: Time in seconds to wait between LEDs.
        """
        color_map = {'green': self.Green, 'red': self.Red, 'yellow': self.Yellow}
        if color not in color_map:
            raise ValueError("Color must be 'green', 'red', or 'yellow'")
        color_list = color_map[color]
        step = 1 if end >= start else -1
        for idx in range(start, end + step, step):
            self.led_color(idx, color_list)
            sleep(delay)
        self.lights_off()

    def led_range_all(self, start=1, end=10, color='red', delay=0.2):
        """
        Light up all LEDs in the range [start, end] cumulatively, so all stay on.
        Args:
            start: Starting LED index.
            end: Ending LED index.
            color: 'green', 'red', or 'yellow'.
            delay: Time in seconds to wait between LEDs.
        """
        color_map = {'green': self.Green, 'red': self.Red, 'yellow': self.Yellow}
        if color not in color_map:
            raise ValueError("Color must be 'green', 'red', or 'yellow'")
        color_list = color_map[color]
        reg3 = reg2 = reg1 = 0
        step = 1 if end >= start else -1
        for idx in range(start, end + step, step):
            if 1 <= idx <= 10:
                l3, l2, l1 = color_list[idx-1]
                reg3 |= l3
                reg2 |= l2
                reg1 |= l1
                self.segment_switch(reg3, reg2, reg1)
                sleep(delay)
        self.segment_switch(reg3, reg2, reg1)

    def level_graph(self, Graph, level, color_ranges=[4,3,3]):
        """
        Display a level graph with customizable color segment sizes.
        The bargraph divides the 10 LEDs into up to three colored sections (green, yellow, red).
        Args:
            Graph: [green_list, yellow_list, red_list] color lookup tables.
            level: int, 1-10 (number of LEDs to light).
            color_ranges: list of three ints, e.g. [4,3,3], specifying the number of green, yellow, and red LEDs.
        """
        if not (1 <= level <= 10):
            self.lights_off()
            return
        green_list, yellow_list, red_list = Graph
        green_count, yellow_count, red_count = color_ranges
        # Compute LED index ranges for each color segment
        green_end = green_count
        yellow_end = green_count + yellow_count
        reg3 = reg2 = reg1 = 0
        for idx in range(1, level + 1):
            if idx <= green_end:
                l3, l2, l1 = green_list[idx-1]
            elif idx <= yellow_end:
                l3, l2, l1 = yellow_list[idx-1]
            else:
                l3, l2, l1 = red_list[idx-1]
            reg3 |= l3
            reg2 |= l2
            reg1 |= l1
        self.segment_switch(reg3, reg2, reg1)

def main():
    # Blink LED to show the start
    led = machine.Pin(25, machine.Pin.OUT) # On-board LED
    for i in range(4):
        sleep(0.1)
        led.value(1)
        sleep(0.1)
        led.value(0)
    sleep(1)
    
    # Inst BarGraph2 class
    spi = SPI(0, baudrate=100000, polarity=0, phase=0, sck=Pin(18), mosi=Pin(19), miso=Pin(16))
    bargraph = BarGraph2(spi, cs_pin=17, mr_pin=20, pwm_pin=2)
    bargraph.reset()
    sleep(.1)
    bargraph.power(POWER_ON)
    sleep(.1)
    bargraph.reset()
    sleep(.1)
    bargraph.segment_switch(0, 0, 0)  # all  LEDs OFF   
    bargraph.led_range(start=1, end=10, color='red', delay=0.05)
    bargraph.led_range(start=10, end=1, color='yellow', delay=0.05)
    bargraph.led_range(start=1, end=10, color='green', delay=0.05)
    bargraph.led_range_all(start=1, end=10, color='red', delay=0.05)
    bargraph.led_range_all(start=10, end=1, color='yellow', delay=0.05)
    bargraph.led_range_all(start=1, end=10, color='green', delay=0.05)
    
    frequency = 1        # 1 Hz
    duration = 10        # seconds to run demo
    sampling_rate = 20   # updates per second
    steps = duration * sampling_rate
    import math
    for i in range(steps):
        t = i / sampling_rate
        # Sine wave in range [-1, 1]
        s = math.sin(2 * math.pi * frequency * t)
        # Map to level in [1, 10]
        level = int(round(5 * s + 5.5))  # 5.5 ensures range is 1..10
        bargraph.level_graph(bargraph.Graph, level, color_ranges=[6,2,2])
        time.sleep(1/sampling_rate)  
    

if __name__ == "__main__":
    main()

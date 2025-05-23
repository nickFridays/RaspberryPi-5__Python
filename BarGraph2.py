# https://github.com/nickFridays/RaspberryPi-5__Python/blob/main/BarGraph2.py
# SN74HC595N shift register   https://www.ti.com/lit/ds/symlink/sn74hc595.pdf
# for Mikroe BarGraph-2       https://www.mikroe.com/bargraph-2-click

from machine import Pin, SPI
import utime
from utime import sleep
from utime import sleep_ms
class BarGraph2:
    LED_SEGMENTS = [0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]
    #LED_SEGMENTS = [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00]
    
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
        #self.reset()
        self.write_byte(reg_3)
        self.write_byte(reg_2)
        self.write_byte(reg_1)

    def led_green(self, index):
           

    def led_red(self, index):
        

    def led_yellow(self, index):
       

    def lights_off(self):
        self.segment_switch(0x00, 0x00, 0x00)

    def power(self, on_off):
        self.pwm.value(on_off)


def main():
    # Blink LED to show the start
    led = machine.Pin(25, machine.Pin.OUT) # On-board LED
    for i in range(5):
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
    bargraph.power(BarGraph2.POWER_ON)
    sleep(.1)
    bargraph.reset()
    sleep(.1)
  
    bargraph.segment_switch(0, 0, 0)  # all  LEDs OFF
    bargraph.segment_switch(0, 0, 0)
    sleep(1)
    

if __name__ == "__main__":
    main()

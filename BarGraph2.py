# SN74HC595N shift register   https://www.ti.com/lit/ds/symlink/sn74hc595.pdf
# for Mikroe BarGraph-2       https://www.mikroe.com/bargraph-2-click


from machine import Pin, SPI
import utime

class BarGraph2:
    LED_SEGMENTS = [0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]
    POWER_ON = 1
    POWER_OFF = 0

    def __init__(self, spi, cs_pin, mr_pin, pwm_pin):
        self.spi = spi
        self.cs = Pin(cs_pin, Pin.OUT)
        self.mr = Pin(mr_pin, Pin.OUT)
        self.pwm = Pin(pwm_pin, Pin.OUT)

        self.cs.value(1)  # Deselect the device
        self.pwm.value(0)  # Power off by default

    def reset(self):
        self.mr.value(0)
        utime.sleep_ms(100)
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

    def led_green(self, index):
        if 1 <= index <= 10:
            reg_1 = self.LED_SEGMENTS[index - 1] if index <= 8 else 0x00
            reg_2 = self.LED_SEGMENTS[index - 9] if index > 8 else 0x00
            self.segment_switch(0x00, reg_2, reg_1)

    def led_red(self, index):
        if 1 <= index <= 10:
            reg_2 = self.LED_SEGMENTS[index - 1] if index <= 6 else 0x00
            reg_3 = self.LED_SEGMENTS[index - 7] if index > 6 else 0x00
            self.segment_switch(reg_3, reg_2, 0x00)

    def led_yellow(self, index):
        if 1 <= index <= 10:
            reg_1 = self.LED_SEGMENTS[index - 1] if index <= 6 else 0x00
            reg_2 = self.LED_SEGMENTS[index + 1] if index <= 6 else 0x00
            reg_3 = self.LED_SEGMENTS[index - 7] if index > 6 else 0x00
            self.segment_switch(reg_3, reg_2, reg_1)

    def lights_out(self):
        self.segment_switch(0x00, 0x00, 0x00)

    def power(self, on_off):
        self.pwm.value(on_off)


# Example usage
def main():
    spi = SPI(0, baudrate=100000, polarity=0, phase=0, sck=Pin(18), mosi=Pin(19), miso=Pin(16))
    bargraph = BarGraph2(spi, cs_pin=17, mr_pin=20, pwm_pin=21)

    bargraph.power(BarGraph2.POWER_ON)
    utime.sleep_ms(100)

    # Single LED switch
    for i in range(1, 11):
        bargraph.led_green(i)
        utime.sleep_ms(200)
        bargraph.led_red(11 - i)
        utime.sleep_ms(200)

    bargraph.lights_out()

    # Multiple LED switch
    for i in range(1, 11):
        bargraph.led_green(i)
    utime.sleep(1)
    for i in range(1, 11):
        bargraph.led_red(i)
    utime.sleep(1)
    for i in range(1, 11):
        bargraph.led_yellow(i)
    utime.sleep(1)

if __name__ == "__main__":
    main()

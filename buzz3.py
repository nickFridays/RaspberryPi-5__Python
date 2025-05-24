"""
Micropython 
Class to control the Mikroe Buzz 3 Click with PAM8904JER via Raspberry Pi Pico.
EN1: Volume control
EN2: Volume control (combination with EN1)
DIN: Sound input (PWM for tone generation)
Buzz 3 Click is installed in Mikroe Click Shield for Pi Pico
"""

from machine import Pin, PWM
import time
import utime
from utime import sleep

class Buzz3:
    # PAM8904JER EN1, EN2 truth table for volume (see datasheet Table 1)
    # 00: -18dB (Mute), 01: -12dB (Low), 10: -6dB (Medium), 11: 0dB (Max)
    VOLUME_LEVELS = {
        'mute':   (0, 0),   # -18dB
        'low':    (0, 1),   # -12dB
        'medium': (1, 0),   # -6dB
        'high':   (1, 1),   #  0dB
    }

    def __init__(self, en1_pin=26, en2_pin=6, din_pin=2):
        # Set up EN1, EN2 as digital outputs
        self.en1 = Pin(en1_pin, Pin.OUT)
        self.en2 = Pin(en2_pin, Pin.OUT)
        # Set up DIN as PWM output
        self.din_pwm = PWM(Pin(din_pin))
        self.din_pwm.duty_u16(0)  # Start silent
        self.set_volume('mute')
        self.set_tone(0)          # No tone

    def set_volume(self, level):
        """
        Adjust the output volume.
        :param level: One of 'mute', 'low', 'medium', 'high'
        """
        if level not in self.VOLUME_LEVELS:
            raise ValueError("level must be one of: " + ", ".join(self.VOLUME_LEVELS.keys()))
        en1_val, en2_val = self.VOLUME_LEVELS[level]
        self.en1.value(en1_val)
        self.en2.value(en2_val)

    def set_tone(self, freq, duty=32768):
        """
        Adjust the sound frequency and pitch.
        :param freq: Frequency in Hz (0 to stop)
        :param duty: Duty cycle (0-65535, default 50%)
        """
        if freq <= 0:
            self.din_pwm.duty_u16(0)  # Stop PWM (silence)
            self.din_pwm.freq(1000)   # Set some default
        else:
            # PAM8904JER supports a wide range, datasheet recommends 20kHz max, common buzzer range 1-10kHz
            self.din_pwm.freq(freq)
            self.din_pwm.duty_u16(duty)

    def beep(self, freq=4000, duration_ms=200, volume='high'):
        """
        Convenience: beep at a given frequency, duration, and volume.
        """
        import time
        prev_volume = self.get_volume_level()
        self.set_volume(volume)
        self.set_tone(freq)
        time.sleep_ms(duration_ms)
        self.set_tone(0)
        self.set_volume(prev_volume)

    def get_volume_level(self):
        """
        Returns the current volume setting as a string.
        """
        en1_val = self.en1.value()
        en2_val = self.en2.value()
        for k, v in self.VOLUME_LEVELS.items():
            if (en1_val, en2_val) == v:
                return k
        return 'unknown'
 
    def play_riff(self, riff, volume='high'):
        """
        General method to play a musical riff.
        :param riff: List of tuples (frequency, duration_ms, pause_after_ms)
        :param volume: 'mute', 'low', 'medium', or 'high'
        """
        prev_volume = self.get_volume_level()
        self.set_volume(volume)
        for note in riff:
            # Support both (freq, duration) and (freq, duration, pause) formats
            if len(note) == 2:
                freq, dur = note
                pause = 0
            else:
                freq, dur, pause = note
            self.set_tone(freq)
            time.sleep_ms(dur)
            self.set_tone(0)
            if pause > 0:
                time.sleep_ms(pause)
        self.set_volume(prev_volume)      

def main():
    # Blink LED to show the start
    led = machine.Pin(25, machine.Pin.OUT) # On-board LED
    for i in range(4):
        sleep(0.1)
        led.value(1)
        sleep(0.1)
        led.value(0)
    sleep(1)
    
    buzz = Buzz3()
    buzz.set_volume('low')
    #buzz.set_tone(440)  # Play A4
    #time.sleep(1)
    buzz.set_tone(0)    # Stop sound
    buzz.set_volume('mute')
    #buzz.beep()

    smokeOnTheWater=[(277,400,120),(330,400,180),(370,600,200),(277,400,120),
                     (330,400,180),(392,250,80),(370,600,200),(277,400,120),
                     (330,400,180),(370,600,200),(330,400,150),(277,800,500)]
    
    sweetChild = [(392, 250, 60), (330, 250, 60), (349, 250, 60), (294, 250, 60),
                  (330, 250, 60), (294, 250, 60), (349, 250, 60), (392, 400, 120)]

    forElise = [(659, 200, 50), (622, 200, 50), (659, 200, 50), (622, 200, 50), 
                    (659, 200, 50),(494, 200, 50), (587, 200, 50), (523, 200, 50), 
                    (440, 400, 120),(0, 200, 50)]

    cha_cha_cha =[(523, 180, 60), (523, 90, 30), (523, 90, 30), (587, 180, 60),
                    (659, 360, 120), (587, 180, 60), (523, 90, 30), (523, 90, 30),
                    (659, 180, 60), (523, 360, 120)]

    buzz.play_riff(forElise, volume='low')
  
  

# Python code for Raspberry Pi 5 to read adc data from Texas Instrument AMC13xM0x.
# Mikroe ISO ADC-7
# On-board Crystal for 8.192 MHz, for  an integer clock mode

from time import sleep
from array import array
import spidev
import pi5gpio

RST = 17         # GPIO connected to the AMC13xM0x pin Reset
DATA_READY = 26  # GPIO Input connected to the AMC13xM0x pin data_ready
SYNC_XEN = 25    # SYNC Clock
# Register and gain constants
GAIN_REG = 0x04  # Register address for gain control
GAINS = {
        "GAIN_0"  : 0x0000, # default 
        "GAIN_1"  : 0x0001,   
        "GAIN_2"  : 0x0002,  
        "GAIN_4"  : 0x0004, 
        "GAIN_8"  : 0x0008,  
        "GAIN_16" : 0x0010,  
        "GAIN_32" : 0x0020, 
        "GAIN_64" : 0x0040, 
        "GAIN_128": 0x0080 
        }

# Class to read adc data from TI AMC13xM0x family.  
class AMC13xM0x:
    def __init__(self, spi_bus=0, spi_device=1,spi_mode=1):
        try:
            self.spi = spidev.SpiDev()
            self.spi.open(spi_bus, spi_device) # Init SPI bus=0 dev=1. Check your system with $ ls /dev/spi*
            self.spi.max_speed_hz = 100000
            self.spi.mode = spi_mode  # SPI mode 
            self.reset()
            # Enable internal DC-DC converter by writing 1 to the DCDC CTRL register bit_0
            self.write_reg(0x31, 1)  
            if self.read_reg(0x31)==0:  # Read the DCDC_CTRL register
                print("ISO ADC-7 AMC13xM0x failed to turn dcdc power on in DCDC_CTRL Register")
            sleep(0.001)
        except Exception as e:
            print(str(e)+ "  ISO ADC-7 Init Error" )
        sleep(0.001)

    def close(self):
        if self.spi:
            self.spi.close()

    def reset(self,rst_gpio=RST):
        # Hardware reset
        gpio_d.set_outputs([rst_gpio],[0])  # GPIO RST (17) to low for reset
        sleep(0.01)
        gpio_d.set_outputs([rst_gpio],[1])  # Set GPIO (17) to high to complete reset
        sleep(0.01)

    def read_raw_adc(self ):
        '''Reads the raw ADC values from the read register'''
        # At reset, default is 24-bit words, 16-bit value plus one byte of 0s. figured out 
        txdata = [0] * 15
        rxdata = self.spi.xfer2(txdata)  # Usage: Transfer data via SPI and capture response
        # The RAW ADC data, returned as a list
        raw_adc = [(rxdata[3] << 8) + rxdata[4], (rxdata[6] << 8) + rxdata[7], (rxdata[9] << 8) + rxdata[10]]
        return raw_adc

    def read_reg(self, reg_addr):
        # Reads a register from the reg_addr - expected address from 0 to 63
        # Check the input address
        if not (0 <= reg_addr <= 63):
            raise ValueError("Register address out of range (0-63).")
        # Read register
        txdata = [0b10100000 | (reg_addr >> 1), (reg_addr & 1) << 7, 0] + [0] * 12
        rxdata = self.spi.xfer2(txdata) # Read SPI register with specified address
        # Send no_op
        txdata2 = [0] * 15
        rxdata2 = self.spi.xfer2(txdata2) 
        return (rxdata2[0] << 8) | rxdata2[1]

    def write_reg(self, reg_addr, reg_value):
        # Write to a register where expected address from 0 to 63 and reg_value - the new value, expect 16-bit
        # Check the input address
        if not (0 <= reg_addr <= 63):
            raise ValueError("Register address out of range (0-63).")
        if not (0 <= reg_value <= 65535):
            raise ValueError("Register value out of range (16-bit).")
        # Write register
        txdata = [0b01100000 | (reg_addr >> 1), (reg_addr & 1) << 7, 0, (reg_value >> 8), (reg_value & 255)] + [0] * 10
        self.spi.xfer2(txdata)  # Write the specified value to the SPI register

    def read_adc_mv(self, ch, smpl_numbr,round_to=4):
        """Reads a number of samples from a specific channel. Returns the rounded average in mV.
        ch: ADC channel #. It depends on the chip (0, 1, or 2).  smpl_numbr: Number of samples to acquire for averaging."""
        if ch not in [0, 1, 2]: ch=0
        samples = []
        for _ in range(smpl_numbr):
            temp = self.read_raw_adc()
            samples.append(temp[ch])  # Read from the specified channel
            sleep(0.001)
        # Calculate average
        avrg = sum(samples) / len(samples)
        # Convert to millivolts for Vref= 1.2V
        if avrg > 32767:  # Handle signed 16-bit integer values
            avrg -= 65536
        mv_volt = (avrg / 32768.0) * 1200  # Convert to mV
        return round(mv_volt,round_to)

    def read_adc_volt(self, ch, smpl_numbr,round_to=6):
        """ Reads ADC data from a specified channel, calculates the rounded average in volts to return.
        It uses array module for faster processing of big numbers of "real time" readings."""
        if ch not in [0, 1, 2]: ch = 0
        samples = array("I", [0] * smpl_numbr)  # Create an array for storing samples
        for i in range(smpl_numbr):
            temp = self.read_raw_adc()
            samples[i] = temp[ch]  # Read from the specified channel
            sleep(0.001)
        # Calculate average
        avrg = sum(samples) / len(samples)
        # Convert to volts for Vref 1.2V
        if avrg > 32767:  # Handle signed 16-bit integer values
            avrg -= 65536
        volt = (avrg / 32768.0) * 1.2  # Convert to volts
        return round(volt,round_to)
    
    def read_funct_regs(self):           # To use for further development
        id_reg     = self.read_reg(0x0)  # ID register. Datasheet may say different
        status_reg = self.read_reg(0x1)  # status register
        mode_reg   = self.read_reg(0x2)  # mode register
        clock_reg  = self.read_reg(0x3)  # clock register
        gain_reg   = self.read_reg(0x4)  # gain register
        cfg_reg    = self.read_reg(0x6)  # configuration register    

    def set_gain(self, gain):
        """Sets a new gain value in the GAIN register. To read 1 mV , max Gain is 4 or overflow """
        if gain not in GAINS:
            gain=1        
        gain_value = GAINS[gain]
        self.write_reg(GAIN_REG, gain_value)

    def get_gain(self):
        """Returs the current gain value stored in the GAIN register."""
        return self.read_reg(GAIN_REG)

    def device_sync(self, io=SYNC_XEN):
        """Sync the device by toggling the GPIO pin with specific timing."""
        gpio_d.set_outputs([SYNC_XEN],[0])  
        sleep(0.00005)  # Wait for 50 us
        gpio_d.set_outputs([SYNC_XEN],[1])  


# Usage example 
if __name__ == "__main__":

    # check imports for pi5gpio.py. Locate it in the same repo
    # Class instance to configure and controll Raspberry Pi-5 GPIO
    gpio_d = pi5gpio.gpioD()  
    gpio_d.config_outputs([RST],[0])   # Reset pin. Configure a GPIO as output and set its initial state to 0
    gpio_d.config_inputs([DATA_READY],[pi5gpio.Bias.AS_IS])  # Data_Ready pin. Configure GPIO as an input with no pull settings
                                                             # if it needs a pull-down [Bias.PULL_DOWN]
    # Init class ISO ADC-7 
    iso7 = AMC13xM0x()
    NUM_SAMPLES = 30
    CHANNEL = 0  # Specify the channel to read from (0, 1, or 2)
    # To read 1 mV , max Gain is 4
    # Read mVolt
    avg_mv = iso7.read_adc_mv(CHANNEL, NUM_SAMPLES)
    print(f"Average ADC Value on Channel {CHANNEL} in mV: {avg_mv}")
    # Read Volt
    avg_volt = iso7.read_adc_volt(CHANNEL, NUM_SAMPLES)
    print(f"Average ADC Value on Channel {CHANNEL} in Volts: {avg_volt}")

    iso7.set_gain("GAIN_0") # 0 no amplification, 1 is double , 2 is quad , 
    print('Gain=',iso7.get_gain())
    
    for gain in GAINS:      # Read 0.5mV with each Gain
        iso7.set_gain(gain) # Gain_0 x1, Gain_1 x2 ,Gain_2 x4 , Gain_4 x16, Gain_8 results in overflow
        sleep(0.01) 
        print('Gain=',iso7.get_gain())
        # Read mVolt
        avg_mv = iso7.read_adc_mv(CHANNEL, NUM_SAMPLES)
        print(f"Average ADC Value on Channel {CHANNEL} in mV: {avg_mv}")

    iso7.close()

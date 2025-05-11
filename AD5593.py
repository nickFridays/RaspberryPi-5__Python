
# Mikroe ADAC Click. AD5593 with 8 configurable channels.

from smbus2 import SMBus, i2c_msg
from time import sleep

# Vectors
CONFIG_MODE   = 0b0000 << 4
DAC_WRITE     = 0b0001 << 4
ADC_READBACK  = 0b0100 << 4
DAC_READBACK  = 0b0101 << 4
GPIO_READBACK = 0b0110 << 4
REG_READBACK  = 0b0111 << 4
# Control Registers
NO_OP_REG      = 0b00000000
ADC_SEQ_REG    = 0b0010
GP_CONTR_REF   = 0b0011
ADC_PIN_CONF   = 0b0100
DAC_PIN_CONF   = 0b0101
PULLDOWN_CONF  = 0b0110
LDAC_MODE      = 0b0111
GPIO_W_CONF    = 0b1000
GPIO_W_DATA    = 0b1001
GPIO_R_CONF    = 0b1010
PWRDWN_REFCONF = 0b1011
OPENDRAIN_CONF = 0b1100
IO_3STATE_PIN  = 0b00001101
SOFT_RESET     = 0b1111
# 3_State
STATE_LOW    = 0
STATE_HIGH   = 1
STATE_H_IMPD = 3

class AD5593:
    def __init__(self, bus, address=0x10, intr_ref=0,vref=5000):
        self._i2c = bus
        self._address = address
        self._data = [0] * 3
        self._intr_ref = intr_ref
        self.vref = vref if intr_ref==0  else 2500
        self.reset()
    # Configure a pin as an output
    def conf_output(self, pin):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | GPIO_W_CONF, 2)
        self._data[0] = CONFIG_MODE | GPIO_W_CONF
        self._data[1] = reg[0]
        self._data[2] = reg[1] | (1 << pin)
        self._write()
        return(pin)
    # Configure a pin as an input
    def conf_input(self, pin):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | GPIO_R_CONF, 2)
        self._data[0] = CONFIG_MODE | GPIO_R_CONF
        self._data[1] = reg[0]
        self._data[2] = reg[1] | (1 << pin)
        self._write()
        return(pin)
    # Configure a pin as a DAC
    def conf_dac(self, pin):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | DAC_PIN_CONF, 2)
        self._data[0] = CONFIG_MODE | DAC_PIN_CONF
        self._data[1] = reg[0]
        self._data[2] = reg[1] | (1 << pin)
        self._write()
        return(pin)
    # Configure a pin as an ADC
    def conf_adc(self, pin, gain=1):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | ADC_PIN_CONF, 2)
        self._data[0] = CONFIG_MODE | ADC_PIN_CONF
        self._data[1] = reg[0]
        self._data[2] = reg[1] | (1 << pin)
        self._write()
        if self._intr_ref == 1:
            # Activate Vref
            reg = self._read(REG_READBACK | PWRDWN_REFCONF, 2)
            self._data[0] = CONFIG_MODE | PWRDWN_REFCONF
            self._data[1] = reg[0] | (1 << 1)
            self._data[2] = reg[1]
            self._write()
            if gain == 2:
                reg = self._read(REG_READBACK | GP_CONTR_REF, 2)
                self._data[0] = CONFIG_MODE | GP_CONTR_REF
                self._data[1] = reg[0]
                self._data[2] = reg[1] | (1 << 5)  # Range 2x Vref
                self._write()
            else:
                reg = self._read(REG_READBACK | GP_CONTR_REF, 2)
                self._data[0] = CONFIG_MODE | GP_CONTR_REF
                self._data[1] = reg[0]
                self._data[2] = reg[1] & ~(1 << 5)  # Range nx Vref
                self._write()
        return(pin)
    # Configure a pin as a three-state pin
    def conf_3_state(self, pin):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | IO_3STATE_PIN, 2)
        self._data[0] = CONFIG_MODE | IO_3STATE_PIN
        self._data[1] = reg[0]
        self._data[2] = reg[1] | (1 << pin)
        self._write()
        return(pin)
    # Set the three-state mode of a pin (high, low, or high-impedance)
    def set_3_state(self, pin, state):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | IO_3STATE_PIN, 2)
        self._data[0] = CONFIG_MODE | IO_3STATE_PIN
        self._data[1] = reg[0]
        if state == STATE_HIGH:
            self._data[2] = reg[1] | (1 << pin)  # Set high
        elif state == STATE_LOW:
            self._data[2] = reg[1] & ~(1 << pin)  # Set low
        elif state == STATE_H_IMPD:
            self._data[2] = reg[1] & ~(1 << pin)  # Set high-impedance (disable output)
        self._write()
    # Set the DAC output in millivolts
    def setDACmVolt(self, pin, value):
        pin = self.validate_pin(pin)
        #dac_value = int((value / self.getVref()) * 4095)  # Convert voltage to DAC value
        dac_value = int(value / self.vref * 4095)
        self._data[0] = DAC_WRITE | pin
        self._data[1] = (dac_value >> 8) & 0xFF
        self._data[2] = dac_value & 0xFF
        self._write()
    # Read the digital input value from specific IO. We call it pin number
    def getInputState(self, pin):
        pin = self.validate_pin(pin)
        self._i2c.write_byte(self._address, GPIO_READBACK)
        msg = self._i2c.read_i2c_block_data(self._address, GPIO_READBACK, 2)
        return (msg[1] >> pin) & 0x01
    # Read the ADC value in millivolts
    def readADCmVolt(self, pin, average=1):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | ADC_SEQ_REG, 2)
        self._data[0] = CONFIG_MODE | ADC_SEQ_REG
        self._data[1] = reg[0] | (1 << 1)  # Activate repetition
        self._data[2] = (1 << pin)
        self._write()
        self._i2c.write_byte(self._address, ADC_READBACK)
        values = [0] * average
        for x in range(average):
            msg = self._i2c.read_i2c_block_data(self._address, ADC_READBACK, 2)
            values[x] = int(((msg[0] & 0x0F) << 8) + msg[1])
        avg_value = int(sum(values) / len(values))
        #vref = self.getVref()
        adc_mv= (avg_value / 4096) * self.vref 
        return adc_mv  # Return value in millivolts
    # Get the reference voltage
    def getVref(self):
        mask = 0b00100000
        reg = self._read(REG_READBACK | GP_CONTR_REF, 2)
        vref = 2500 * (((reg[1] & mask) >> 5) + 1) 
        return vref
    # Set the internal reference voltage
    def setVref(self, activate):
        if activate:
            reg = self._read(REG_READBACK | PWRDWN_REFCONF, 2)
            self._data[0] = CONFIG_MODE | PWRDWN_REFCONF
            self._data[1] = reg[0] | (1 << 1)  # Activate Vref
            self._data[2] = reg[1]
            self._write()
        else:
            reg = self._read(REG_READBACK | PWRDWN_REFCONF, 2)
            self._data[0] = CONFIG_MODE | PWRDWN_REFCONF
            self._data[1] = reg[0] & ~(1 << 1)  # Deactivate Vref
            self._data[2] = reg[1]
            self._write()
    # Read the voltage at a specified pin in volts. Returns: - float
    def readVoltage(self, pin, average=3):
        pin = self.validate_pin(pin)
        return self.readADCmVolt(pin, average) / 1000  # Return the value in volts
    # Set the output value of a pin
    def setOutput(self, pin, value=0):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | GPIO_W_DATA, 2)
        self._data[0] = CONFIG_MODE | GPIO_W_DATA
        self._data[1] = NO_OP_REG
        if value:
            self._data[2] = reg[1] | (1 << pin)  # 1
        else:
            self._data[2] = reg[1] & ~(1 << pin)  # 0
        self._write()
    # Toggle the output value of a pin
    def toggle(self, pin):
        pin = self.validate_pin(pin)
        reg = self._read(REG_READBACK | GPIO_W_DATA, 2)
        self._data[0] = CONFIG_MODE | GPIO_W_DATA
        self._data[1] = NO_OP_REG
        self._data[2] = reg[1] ^ (1 << pin)
        self._write()
    # Power up or down all pins
    def powerAll(self, state):
        if state:  # Powering up
            reg = self._read(REG_READBACK | PWRDWN_REFCONF, 2)
            self._data[0] = CONFIG_MODE | PWRDWN_REFCONF
            self._data[1] = reg[0] & ~(1 << 2)  # Activate Vref
            self._data[2] = reg[1]
            self._write()
        else:  # Powering down
            reg = self._read(REG_READBACK | PWRDWN_REFCONF, 2)
            self._data[0] = CONFIG_MODE | PWRDWN_REFCONF
            self._data[1] = reg[0] | (1 << 2)
            self._data[2] = reg[1]
            self._write()
    # Perform a soft reset of the device
    def reset(self):
        self._data[0] = CONFIG_MODE | SOFT_RESET
        self._data[1] = 0x0D
        self._data[2] =  0    # 0xAC through exception
        self._write()
    # Validate the pin number
    def validate_pin(self, pin):
        if not 0 <= pin <= 7:
            raise ValueError('Invalid pin {}. Use 0-7.'.format(pin))
        return pin
    # Read a register value
    def read_reg(self, reg):
        return bin(self._read(reg, 2))
    # Write a value to a register
    def write_reg(self, reg, msb, lsb):
        self._data[0] = reg
        self._data[1] = msb
        self._data[2] = lsb
        self._write()
    # Read values from the device
    def readValues(self):
        return self._i2c.read_i2c_block_data(self._address, 0, 2)
    # Internal method to read from I2C
    def _read(self, reg=NO_OP_REG, nbBytes=2):
        try:
            self._i2c.write_byte(self._address, reg)
            return self._i2c.read_i2c_block_data(self._address, reg, nbBytes)
        except Exception as e:
            print("AD5593 Error: ",str(e)) 
            #logReport.logger.error("AD5593 Error: ", e)
    # Internal method to write to I2C
    def _write(self):
        try:
            self._i2c.write_i2c_block_data(self._address, self._data[0], self._data[1:])
        except Exception as e:
            print("AD5593 Error: ",str(e)) 
            #logReport.logger.error("AD5593 Error: ", e)


# Usage 
bus = SMBus(1)

import pi5gpio
gpio_d = pi5gpio.gpioD()
gpio_d.config_outputs([19], [0])
gpio_d.set_output(19,1)
gpio_d.set_output(19,1)
sleep(0.3)
# Create an instance of the AD5593 class
ad5593 = AD5593(bus)
ad5593.reset()
# Set pin 0 as output
resp=ad5593.conf_output(pin=0)
# Config pin 1 as Input
ad5593.conf_input(pin=1)
# Set IO 0 output to ON (1)
pin_0=0
ad5593.setOutput(pin_0, 1)
print("Output_0: ", '= 1')
sleep(0.1)
status = ad5593.getInputState(1)
print("Input_1: ",status)
ad5593.setOutput(pin_0, 0)
print("Output_0: ", '= 0')
sleep(0.1)
status = ad5593.getInputState(1)
print("Input_1: ",status)
  
# Set pin 2 and 3 as DAC  pin 4 as ADC
ad5593.conf_dac(2)
ad5593.conf_dac(3)
ad5593.conf_adc(4,gain=2)   #gain=1
sleep(0.2)
# DAC generates 3 mV
ad5593.setDACmVolt(3, 20) 
sleep(0.2)
# ADC reads 3 mV
adc_value = ad5593.readADCmVolt(4)
print(f"ADC value from pin 4: {adc_value}")

ad5593.setDACmVolt(3, 50)
sleep(0.2)
adc_value = ad5593.readADCmVolt(4)
print(f"ADC value from pin 4: {adc_value}")

ad5593.reset()
# Close the I2C bus
bus.close()

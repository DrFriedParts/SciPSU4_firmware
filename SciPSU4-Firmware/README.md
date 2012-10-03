# NOTES

## Configuration Notes

* Assumes 32k XTAL (for accurately calibrating the internal RC oscillator) is not installed on PCB (if populated will need to update firmware)



## Resource Allocations

### PORT C
* Timer0 -- RTOS Loop Timer (2ms Period)
* UART0 -- Control Serial Port (to PC via USB)
* UART1 -- Data Serial Port (to PC via USB)

### PORT F
* Timer0 -- LED Dimming & Audio Volume
* Timer1 -- FAN0, FAN1
* UART1  -- LCD

# TODO

1. Front Panel Mute Switches 

1. Front Panel LEDs

1. Front Panel Rotary

1. Power Control

1. LCD (Serial Port)

1. Fans

1. Thermocouple

## Main Power
1. Control output mute (relay)
2. Confirm actual electrical output
3. Voltage Sensing (ADC + Resolution adjust)
4. Current Sensing (ADC + Resolution adjust)

## Adjustable Channels
1. SPI driver for Resistors
2. EEPROM storage of changes





## DONE

1. System clock

1. RTOS Timer

1. Internal LED's

1. Audio (Beeper)
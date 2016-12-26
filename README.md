SciPSU4_firmware
================
Multi-channel Digitally-controlled low-noise power supply for scientific applications

Demonstration project for CircuitHub, Inc.
Jonathan Friedman, PhD

# MOVED TO getscale/getscale-neurolabware/SciencePSU/SciencePSU-Firmware

## Configuration Notes

### Hardware Options

* Assumes 32k XTAL (for accurately calibrating the internal RC oscillator) is not installed on PCB (if populated will need to update firmware)

### Fuses

* JTAGEN = FALSE
* SUT = 64ms
* BODLVL = 2.9V
* BODACT = Continuously
* BODPD = Continuously
* EESAVE = TRUE


### Resource Allocations

#### PORT C
* Timer0 -- RTOS Loop Timer (2ms Period)
* UART0 -- Control Serial Port (to PC via USB)
* UART1 -- Data Serial Port (to PC via USB)

#### PORT F
* Timer0 -- LED Dimming & Audio Volume
* Timer1 -- FAN0, FAN1
* UART1  -- LCD


## To Do

### Misc.
1. Fans
1. Thermocouple
1. Computer Command Interface

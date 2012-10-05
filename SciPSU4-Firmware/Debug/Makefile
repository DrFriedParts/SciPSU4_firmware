################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../adc.c \
../brain.c \
../fp.c \
../lcd.c \
../lcd_console.c \
../lcd_touch.c \
../pwr_main.c \
../main.c \
../quadrature.c \
../thermal.c \
../uart.c \
../uart_buffer.c \
../ui.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
adc.o \
brain.o \
fp.o \
lcd.o \
lcd_console.o \
lcd_touch.o \
pwr_main.o \
main.o \
quadrature.o \
thermal.o \
uart.o \
uart_buffer.o \
ui.o


OBJS_AS_ARGS +=  \
adc.o \
brain.o \
fp.o \
lcd.o \
lcd_console.o \
lcd_touch.o \
pwr_main.o \
main.o \
quadrature.o \
thermal.o \
uart.o \
uart_buffer.o \
ui.o


C_DEPS +=  \
adc.d \
brain.d \
fp.d \
lcd.d \
lcd_console.d \
lcd_touch.d \
pwr_main.d \
main.d \
quadrature.d \
thermal.d \
uart.d \
uart_buffer.d \
ui.d


C_DEPS_AS_ARGS +=  \
adc.d \
brain.d \
fp.d \
lcd.d \
lcd_console.d \
lcd_touch.d \
pwr_main.d \
main.d \
quadrature.d \
thermal.d \
uart.d \
uart_buffer.d \
ui.d


OUTPUT_FILE_PATH +=SciPSU4-Firmware.elf

OUTPUT_FILE_PATH_AS_ARGS +=SciPSU4-Firmware.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

# AVR32/GNU C Compiler



























./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : (AVR_8_bit_GNU_Toolchain_3.4.0_663) 4.6.2
	$(QUOTE)C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.0.65\AVRToolchain\bin\avr-gcc.exe$(QUOTE) -funsigned-char -funsigned-bitfields -O1 -fpack-struct -fshort-enums -g2 -Wall -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -mmcu=atxmega128a1   -o"$@" "$<" 
	@echo Finished building: $<



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : (AVR_8_bit_GNU_Toolchain_3.4.0_663) 4.6.2
	$(QUOTE)C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.0.65\AVRToolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="SciPSU4-Firmware.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group  -mmcu=atxmega128a1  
	@echo Finished building target: $@
	"C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.0.65\AVRToolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "SciPSU4-Firmware.elf" "SciPSU4-Firmware.hex"
	"C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.0.65\AVRToolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "SciPSU4-Firmware.elf" "SciPSU4-Firmware.eep" || exit 0
	"C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.0.65\AVRToolchain\bin\avr-objdump.exe" -h -S "SciPSU4-Firmware.elf" > "SciPSU4-Firmware.lss"
	"C:\Program Files\Atmel\Atmel Studio 6.0\extensions\Atmel\AVRGCC\3.4.0.65\AVRToolchain\bin\avr-size.exe" "SciPSU4-Firmware.elf"
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS)$(C_DEPS_AS_ARGS) $(EXECUTABLES) 
	rm -rf "SciPSU4-Firmware.elf" "SciPSU4-Firmware.a" "SciPSU4-Firmware.hex" "SciPSU4-Firmware.lss" "SciPSU4-Firmware.eep" "SciPSU4-Firmware.map"
	
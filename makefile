#General makefile for UWRT Mars Rover 2018

#these files have to be on the path
CC=arm-none-eabi-gcc
CPP=arm-none-eabi-g++
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
SIZE=arm-none-eabi-size

APP_PATH = $(BASE_PATH)/app/$(APP_FOLDER)
BUILD_PATH = $(APP_PATH)/build
PROJ_PATH = $(BUILD_PATH)/$(PROJECT)
LIB_PATH = $(BASE_PATH)/lib

#os dependent code
ifeq ($(OS),Windows_NT)
REMOVE_PROGRAM = del /S  $(subst /,\,$(BASE_PATH))\*.o
REMOVE_FOLDER = rmdir /s/q
GCC_INC = -IC:/Program\ Files\ (x86)/GNU\ Tools\ ARM\ Embedded/4.9\ 2015q3/arm-none-eabi/include\

else
REMOVE_PROGRAM = rm -rf
REMOVE_FOLDER =
GCC_INC =
endif

MBED = $(LIB_PATH)/mbed

APP_SRC_C = $(wildcard $(APP_PATH)/src/*.c)
LIB_SRC_C = $(wildcard $(LIB_PATH)/*/src/*.c)

APP_SRC_CPP = $(wildcard $(APP_PATH)/src/*.cpp)
LIB_SRC_CPP = $(wildcard $(LIB_PATH)/*/src/*.cpp)

APP_INC = -I$(APP_PATH)/inc
LIB_INC = $(addprefix -I,$(wildcard $(LIB_PATH)/*/inc))

MBED_SRC_C = $(wildcard $(MBED)/hal/*.c) \
			 $(wildcard $(MBED)/hal/TARGET_FLASH_CMSIS_ALGO\*.c) \
			 $(wildcard $(MBED)/platform/*.c) \
			 $(wildcard $(MBED)/targets/TARGET_STM/*.c) \
			 $(wildcard $(MBED)/targets/TARGET_STM/TARGET_STM32F0/*.c) \
			 $(wildcard $(MBED)/targets/TARGET_STM/TARGET_STM32F0/device/*.c) \
			 $(wildcard $(MBED)/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/*.c) \
			 $(wildcard $(MBED)/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/*.c)

MBED_SRC_CPP = $(wildcard $(MBED)/drivers/*.cpp) \
			   $(wildcard $(MBED)/platform/*.cpp)

MBED_INC = -I$(MBED) \
		   -I$(MBED)/cmsis \
		   -I$(MBED)/cmsis/TARGET_CORTEX_M \
		   -I$(MBED)/cmsis/TARGET_CORTEX_M/TOOLCHAIN_GCC \
		   -I$(MBED)/drivers \
		   -I$(MBED)/hal \
		   -I$(MBED)/hal/storage_abstraction \
		   -I$(MBED)/hal/TARGET_FLASH_CMSIS_ALGO \
		   -I$(MBED)/platform \
		   -I$(MBED)/targets/TARGET_STM \
		   -I$(MBED)/targets/TARGET_STM/TARGET_STM32F0 \
		   -I$(MBED)/targets/TARGET_STM/TARGET_STM32F0/device \
		   -I$(MBED)/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC \
		   -I$(MBED)/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device


# Location of the linker scripts
LDSCRIPT_INC = $(MBED)/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/TOOLCHAIN_GCC_ARM

###################################################

CFLAGS  = -Wall -g -Os
CFLAGS += -mlittle-endian -mcpu=cortex-m0  -march=armv6-m -mthumb
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJECT).map
CFLAGS += -u _printf_float -u _scanf_float

LFLAGS = $(CFLAGS) -lm

###################################################

vpath %.c src

# add startup file to build
SRCS_C = $(APP_SRC_C) $(LIB_SRC_C) $(MBED_SRC_C)
SRCS_CPP = $(APP_SRC_CPP) $(LIB_SRC_CPP) $(MBED_SRC_CPP)
SRCASM = $(MBED)/targets/TARGET_STM/TARGET_STM32F0/TARGET_NUCLEO_F091RC/device/TOOLCHAIN_GCC_ARM/startup_stm32f091xc.S
INC = $(APP_INC) $(LIB_INC) $(MBED_INC)

# need if you want to build with -DUSE_CMSIS
#SRCS += stm32f0_discovery.c
#SRCS += stm32f0_discovery.c stm32f0xx_it.c

OBJS = $(SRCS_C:.c=.o) $(SRCS_CPP:.cpp=.o) $(SRCASM:.S=.o)
DEFINES = -DTARGET_STM -DTARGET_STM32F0 -DTARGET_NUCLEO_F091RC -DTARGET_STM32F091RC \
		  -DTARGET_M -DTARGET_CORTEX_M -D__CORTEX_M0 -DARM_MATH_CM0 \
		  -DTARGET_LIKE_CORTEX_M0 -DTARGET_M0 \
		  -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -DMBED_BUILD_TIMESTAMP=1423704949.72 -D__MBED__=1 \
		  -DDEVICE_ANALOGIN -DDEVICE_ANALOGOUT -DDEVICE_CAN -DDEVICE_I2C -DDEVICE_I2CSLAVE \
		  -DDEVICE_I2C_ASYNCH -DDEVICE_INTERRUPTIN -DDEVICE_LOWPOWERTIMER -DDEVICE_PORTIN \
		  -DDEVICE_PORTINOUT -DDEVICE_PORTOUT -DDEVICE_PWMOUT -DDEVICE_RTC -DDEVICE_SERIAL \
		  -DDEVICE_SERIAL_ASYNCH -DDEVICE_SERIAL_FC -DDEVICE_SLEEP -DDEVICE_SPI -DDEVICE_SPISLAVE \
		  -DDEVICE_SPI_ASYNCH -DDEVICE_STDIO_MESSAGES

#os dependent code
ifeq ($(OS),Windows_NT)
CLEAN_PATH  = $(addsuffix ", $(addprefix ", $(subst /,\,$(OBJS))))
CLEAN_FOLDER_PATH = "$(BUILD_PATH)"
REMOVE_OBJS = $(REMOVE_PROGRAM)
else
CLEAN_PATH = $(BUILD_PATH) $(APP_PATH)/*.map $(OBJS)
CLEAN_FOLDER_PATH =
REMOVE_OBJS = $(REMOVE_PROGRAM) $(CLEAN_PATH)
endif


###################################################

.PHONY: proj clean

all: proj

proj:     $(PROJ_PATH).elf

$(PROJ_PATH).elf: $(OBJS)
	@echo Building Project...
	@-mkdir build
	@$(CC) $(LFLAGS) $^ -L$(LDSCRIPT_INC) -TSTM32F091XC.ld -lm -lc -lg -lnosys -lgcc -lstdc++ --specs=nosys.specs -o $@
	$(OBJCOPY) -O ihex $(PROJ_PATH).elf $(PROJ_PATH).hex
	$(OBJCOPY) -O binary $(PROJ_PATH).elf $(PROJ_PATH).bin
	$(OBJDUMP) -St $(PROJ_PATH).elf >$(PROJ_PATH).lst
	$(SIZE) $(PROJ_PATH).elf


%.o: %.c
	@echo $@
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) -std=gnu99 -c -o $@ $<

%.o: %.cpp	
	@echo $@
	@$(CPP) $(CFLAGS) $(INC) $(DEFINES) -std=c++11 -c -o $@ $<

%.o: %.S
	@echo $@
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) -c -o $@ $<


#implement with texane on linux and stmlink CLI on windows
#program: $(PROJECT).bin
#    openocd -f $(OPENOCD_BOARD_DIR)/stm32f0discovery.cfg -f $(OPENOCD_PROC_FILE) -c "stm_flash `pwd`/$(PROJECT).bin" -c shutdown

clean:
	$(REMOVE_OBJS)
	$(REMOVE_FOLDER) $(CLEAN_FOLDER_PATH)
#"src\*.o" "$(HAL)\startup\*.o"

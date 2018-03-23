# MarsRover2018-firmware

Firmware for the 2018 University of Waterloo Mars Rover. Will contain:
- mbed library
- additional libraries written by the team
- application code running on each board

## Board: STM32F091 / NUCLEO-F091RC

## UWRT Firmware Development Instructions

1. Download source code 
    
    `git clone https://github.com/uwrobotics/MarsRover2018-firmware.git`

2. Download toolchain (gcc and make)
   
   For Ubuntu 16.04
    - `sudo apt-get install gcc-arm-none-eabi`
		
	For Windows
    - download [make for windows](http://gnuwin32.sourceforge.net/packages/make.htm) (choose Complete package, except sources)
    - download [gcc-arm-none-eabi for windows](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) (windows 32 bit)
    - add gcc .exe files to path (usually `C:\Program Files (x86)\GNU Tools ARM Embedded\<version>\bin`)
    - add make.exe to path (usually `C:\Program Files (x86)\GnuWin32\bin`)
	
	For Mac
    - Open Command Line
    - Install Homebrew if not installed 
    	`/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
    - Download auto-run script, which will auto install <arm-none-eabi-gcc> with latest version
    	`brew tap ARMmbed/homebrew-formulae`
    - Install <arm-none-eabi-gcc> via HomeBrew
    	`brew install arm-none-eabi-gcc`
	
3. change directory into app/blinky

    `cd MarsRover2018-firmware/app/blinky`
4. run make

    `make`
5. Deploy onto board

	For Ubuntu 16.04
		
    - Install libusb `sudo apt install libusb-1.0-0-dev`
    - Drag and Drop .bin file into NODE_F091RC folder
	
	For Windows
    
    - Download [st-link utility](http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html). Scroll down to Get Software
    - connect USB to nucleo board and open st-link utility
    - load code by going to Target->Program and browse for .bin file
	
	For Mac
    - Drag and Drop .bin file into NODE_F091RC disk (Will show up like other usb devices after connecting) or run `cp build/blinky_out.bin /Volumes/NODE_F091RC/`

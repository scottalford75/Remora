# cakeslob Remora thing - trying to do stuff he doesnt understand, but will try anyways

Things on the go but arent working:
  - SKR3 STM32H743 : starts with the bootloader and then everything else
  - MANTA M8P STM32G0B1 : almost everything working , dmamux and comms thing i dont understand
  - MKS MONSTER8/ROBIN V3.1/SKIPR STM32F407 : With bootloader, bootloader is turning off spi after boot. Without bootloader, it works
  - Fysetc Spider King STM32F407 : works so far, without bootloader

Things working now:
- BTT Octopus STM32F446 working, with bootloader
- Fysetc Spider STM32F446 working, with bootloader

# Remora

The full documentation is at <https://remora-docs.readthedocs.io/en/latest/>

Remora is a free, opensource LinuxCNC component and Programmable Realtime Unit (PRU) firmware to allow LPC17xx and STM32F4 base controller boards to be used in conjuction with a Raspberry Pi to implement a LinuxCNC based CNC controller.

Having a low cost and accessable hardware platform for LinuxCNC is important if we want to use LinuxCNC for 3D printing for example. Having a controller box the size of the printer itself makes no sense in this applicatoin. A SoC based single board computer is ideal in this application. Although developed for 3D Printing, Remora (and LinuxCNC) is highly flexible and configurable for other CNC applications.

Remora has been in use amd development since 2017. Starting on Raspberry Pi 3B and 3B+ eventhough at the time it was percieved that the Raspberry Pi was not a viable hardware for LinuxCNC.

With the release of the RPi 4 the LinuxCNC community now supports the hardware, with LinuxCNC and Preempt-RT Kernel packages now available from the LinuxCNC repository. This now greatly simplifies the build of a Raspberry Pi based CNC controller.

# esp32cam-tinycamera
Tiny camera using ESP32-CAM

For chinese build log: [hackmd](https://hackmd.io/kbnAIwX7QSOUOhwWzUI-vA)
Migrating to English ver. here soon. Work in progress...

Required modules for this build:
* An ESP32-CAM with horizontal OV2640 camera module
* An SPI screen compatible with TFT_eSPI library.
* A tiny li-ion battery that squeeze inside (602530 fits OK)
* An LDO module with very low dropout voltage that can directly convert li-ION battery to 3.3V
* One 3-pole switch to switch between power on and charging
* One button for taking pictures

PCB design accidentally deleted, recreating soon.(some hints still left in hackmd)

# Update Feb 2023
It's hard to find horizontal OV2640 module. this current build seems impossible.
I only have one bought few years ago. Good luck if you have one.
This means the build with horizontal camera module and 1.8-inch screen is finalized.

Another test build with veritical camera and RPi-ili9486 is tested but result in a very poor refrash rate.
Fallback to old setup and another 3D case is required.

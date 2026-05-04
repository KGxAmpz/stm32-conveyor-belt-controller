# STM32 Conveyor Belt Controller

## Overview

This project is an STM32-based conveyor belt controller developed for ECE 4510: Microcontroller Applications. The system was designed to interface with a Fischertechnik-style conveyor belt setup and control belt speed and operation using STM32 timers, PWM generation, input capture, GPIO interrupts, LEDs, a buzzer, simulated IR sensor inputs, and an H-bridge motor driver interface.

## Main Features

- Start and stop control using GPIO external interrupts
- Emergency-stop state handling
- Green start LED and stop LED outputs
- PWM buzzer control using TIM2
- Conveyor belt PWM output using TIM4 Channel 3
- Encoder/frequency input capture using TIM3 Channel 2
- Additional input capture setup using TIM5 Channel 2
- Frequency-based PWM duty-cycle adjustment
- Simulated IR sensor inputs using GPIO pins
- STM32 HAL-based peripheral configuration

## Hardware / Platform

Target platform from the report:

- STM32F429ZI-class microcontroller configuration
- HSE input frequency: 8 MHz
- System clock: 120 MHz
- APB1/APB2 timer clock: 60 MHz
- TIM3 Channel 2 input capture on PA7
- TIM4 Channel 3 PWM output on PD14
- Start/stop inputs on PF0 and PF1
- Sensor inputs on PD0 and PD1
- Start/stop LEDs on PF2 and PF3
- SPI4 configured but not fully used in the final implementation


## How to Use

This is not a full STM32CubeIDE or IAR project export.

To rebuild the full embedded project:

1. Create a new STM32CubeIDE, STM32CubeMX, or IAR project for the matching STM32F429 target.
2. Configure the same timers, GPIO pins, clock tree, and interrupts used in the report.
3. Copy `main.c` into the generated project.
4. Copy `stm32f4xx_it.c` into the generated project.
5. Make sure the generated `main.h`, HAL drivers, startup file, linker script, and CMSIS files are present.
6. Build and flash using the STM32 toolchain.

## Notes

## Author

Kyle Gilbert  
Computer Engineering  
Western Michigan University

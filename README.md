
# Alarm System on KL05z32

The goal of the project was to create an alarm system that utilizes an ultrasonic sensor and an accelerometer to detect movement nearby and changes in the position of the board running the program.

The system can be armed and disarmed by entering a password using a keypad. When the alarm is activated, a red LED lights up, and the WSR-04489 module generates sound.

## System Components:

- FRDM-KL05Z microcontroller
- HC-SR04P ultrasonic distance sensor
- HW-834 matrix keypad
- WSR-04489 module
- LCD1602 display


## Connection of pins


| HC-SR04P ultrasonic distance sensor|    KL05z32             |
| :--------------------------------- | :----------------------|
| `VCC` | `3.3` |
| `GND` | `GND` |
| `Echo_TX_SDA ` | `PTB0  ` |
| `Trig_RX_SCL_I/O ` | `PTB11` |

| HW-834 matrix keypad|    KL05z32             |
| :-------------------| :----------------------|
| `C4`| `PTA6` |
| `C3`| `PTA5` |
| `C2`| `PTA8` |
| `C1`| `PTA9` |
| `R2`| `PTA12` |
| `R3`| `PTA7` |
| `R4`| `PTA11` |

| WSR-04489 module|    KL05z32             |
| :---------------| :----------------------|
| `+5V `| `+5V ` |
| `GND`| `GND` |
| `VCC`| `PTB8` |

| LCD1602 display|    KL05z32             |
| :---------------| :----------------------|
| `VCC`| `+5V` |
| `GND`| `GND` |
| `SDA `| `PTB4 ` |
| `SCL`| `PTB3` |






## Circut Diagram

![](https://github.com/Jbanach123/Alarm_System_KL05z32/blob/6a1da25fc66604cf8819d172ea0c1bbd6ff27ef6/Circut%20diagram.png)


## Operation Description

After arming the system by entering the correct password, the accelerometer and ultrasonic distance sensor enter a standby mode, ready to detect changes.

- The accelerometer, operating based on the INT2 interrupt, compares position changes against a predefined threshold. If the threshold is exceeded, the alarm is triggered.
- For the ultrasonic sensor:
  Pin PTB11 sends a TRIG pulse to the HC-SR04 sensor, initiating measurement.
  Pin PTB13 receives the ECHO signal (Input Capture) from the sensor, measuring the duration of the reflected pulse.
  Pin PTB0 functions as EXTRG_IN, synchronizing measurement triggering within the microcontroller.
If the sensor detects an object within a set distance, the alarm is activated—the red LED lights up, and the buzzer module generates a sound signal.

## User Manual

At startup, the display shows the message "Alarm Off", along with the entered password value (default: 0), and a green LED is lit on the board.

Keypad Functions:
- S1–S9 correspond to digits 1–9
- S10 represents digit 0
- S11 is used to enter password input mode
Pressing S11 turns on the blue LED, indicating entry into administrator mode. The user must then enter the password (default: 801). Upon successful arming, all LEDs turn off.

If the accelerometer or motion sensor detects an unauthorized event, the alarm is activated—the red LED lights up, a sound signal is triggered, and the display shows "Alarm On".

To deactivate the alarm, the user must re-enter administrator mode (press S11) and enter the correct password.

⚠️ Note: After the first program upload, it may be necessary to disconnect and reconnect the board to the power supply.

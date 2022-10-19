# Code for a clock of the ADClock

The ADClock consists of 24 individual clocks.
The special feature of the individual clocks is that the hands can be moved forwards and backwards independently of one another using two stepper motors.
A Microcontroller is built into every watch to control and calibrate the motors moving the hands.
The Arduino Atmega328p receives the instruction for both motors within a single tick over four parallel data wires.
The tick is transmitted by an additional clock wire that is connected to an interrupt pin.

All clocks are connected in serial.
Further data packets, which are sent directly after your own step instruction, are passed on to the next clock.

## The concepts behind this clock

To give you an idea of this ingenious project we provided some pictures of the model and the actual clock.
We will add a link in the future. I guess.

A single clock consists of

- 3D printed parts
  - 1 clock core body in white
  - 2 hands in black (hour and minute)
  - Multiple non visible parts: gears, motor bracket, clips for cable management
- 2 stepper motors to drive the hands forwards and backwards
- 2 hall sensors for calibrating the motors
- 3 bearings for smooth rotation
- 1 metal rod for driving the outer hand
- 1 self designed PCB holding the electronics and exposing the connectors
  - Arduino Atmega328p microcontroller
  - Some crystals and other stuff to make it work
  - A lot of janky solder joints

### Communication

Every clock receives with every tick its next step instruction.
Due to the serial connection of all clocks the clock might need to pass the following instructions to the next clock.

The step instructions consists of four bits.
The first two bits are for the hour hand and the other bits are for the minute hand.

Within the two bits the following four states are coded:

- `00` Keep the hand still
- `10` Rotate the hand backwards
- `01` Rotate the hand forwards
- `11` Calibrate the hand (set's it to 12 o'clock)

As mentioned before a step instruction is transmitted with a single tick over four parallel wires.
An additional wire is connected to the interrupt pin that transmits the ticks.

With this method a lightning fast data transmission is almost guaranteed.

### Stepper Motors

The maximum speed of the stepper motors is one step every 4ms.
This might increase with correctly hooked up power delivery.

The stepper motors are connected with four wires to the controllers allowing it to control each coil independently.
In order to keep the controllers alive the stepper motors are connected to a separate power train.

See next section for a detailed description of the calibration process.

### Calibration

In order to calibrate the stepper motors for each motor there is a hall sensor mounted inside the clock.
A little magnet mounted to the gear moving the hand can enter the field of the hall sensor and can be measured.

The calibration is handled inside the `Calibration.h` and `Calibration.cpp` file as described in the following section.
Both motors are calibrated at the same time.

In order to find for the magnetic field the motor is rotating forwards and counts the steps. (FINDMAGNET)

1. The magnet is found in less than MIN_STEPS_OUTSIDE_FIELD steps:
   1. Rotate the motor backwards until it leaves the magnetic field and continue for at least MIN_STEPS_OUTSIDE_FIELD. (LEAVEMAGNET)
   2. Continue with FINDMAGNET.
2. The magnet is found in more than MIN_STEPS_OUTSIDE_FIELD steps:
   1. The motor rotates forwards through the magnetic field and counts the steps. (INFIELD)
   2. After leaving the magnetic field the counted steps are divided by two and the motor is rotating backwards. (CENTERING)
   3. When finished the motor is calibrated exactly on 12 o'clock. (CALIBRATED)
3. The magnet is not found after 3 full rotations:
   1. The process is canceled.

After the initial calibration the motor is continuously recalibrated.
Every time the motor steps through the magnetic field the position is adjusted so 12 o'clock is always straight up.
Don't know, if that is looking cool and works as intended.
Only time can tell.

### Controller

Tasks of the controller:

- Calibration
- Data communication (read own instruction and pass on the other instructions)
- Executing step instruction
- Recalibration

Right after start the controller calibrates both stepper motors.
While this process is active the controller does not listen to step instructions.

If the step instructions is `11` the hand is calibrated.
Currently only both hands can be calibrated at once.
In the future it might be useful to calibrate them separately to correct single potential wrong moving hands.

# MessageFromInput

!syntax description /UserObjects/MessageFromInput

## Description

This user object provides an option to print a message to the screen during the simulation. The message can be written in the input file. The user can customize *when* the message should print, the default option is to print it at the initialization of the simulation, with `execute_on = INITIAL`.

## Example Input Syntax

!listing test/tests/userobjects/message_from_input/message_from_input.i block=UserObjects

!syntax parameters /UserObjects/MessageFromInput

!syntax inputs /UserObjects/MessageFromInput

!syntax children /UserObjects/MessageFromInput

# CompositionDT

!syntax description /Executioner/TimeStepper/CompositionDT

## Description

The `CompositionDT` TimeStepper takes TimeSteppers in the input file and generates a composed time step size. It always produces the minimum time step size value of all the input TimeSteppers. The hit option ensures the TimeStepper hits the time specified by user input time sequence steppers. All together, The `CompositionDT` TimeStepper allows users to have a base time step with upper or lower limits and hit specified time points.

## Example Input Syntax

!listing test/tests/time_steppers/minimum_all_dt/minimum_all_dt.i block=Executioner



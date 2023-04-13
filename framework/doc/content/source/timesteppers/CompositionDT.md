# CompositionDT

!syntax description /Executioner/TimeStepper/CompositionDT

## Description

The `CompositionDT` TimeStepper takes TimeSteppers as inputs and generates a composed time step size based on the user specific composition rules. A base TimeStepper is required to provide a baseline time step size. The available composition rules including max, min and hit. The max/min option is specified with [!param](/Executioner/TimeStepper/CompositionDT/maximum_step_from). It produces the maximum/minimum time step size value of all the input TimeSteppers. The hit option ensures the TimeStepper hits user specified time. It requires a TimeSequenceStepper with [!param](/Executioner/TimeStepper/CompositionDT/times_to_hit_timestepper) to specify the time sequence to hit. All together, The `CompositionDT` TimeStepper allows users to have a base time step with upper or lower limits and hit specified time points.

## Example Input Syntax

!listing test/tests/time_steppers/composition_dt/composition_dt.i block=Executioner

!syntax parameters /Executioner/TimeStepper/CompositionDT

!syntax inputs /Executioner/TimeStepper/CompositionDT

!syntax children /Executioner/TimeStepper/CompositionDT

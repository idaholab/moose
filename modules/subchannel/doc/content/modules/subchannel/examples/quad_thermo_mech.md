# Thermo-mechanical coupling of an one pin problem

This is an example case of coupling a subchannel calculation (SCM) with a 2D-RZ thermo_mechanical model of a fuel pin.


## Example Description

The example models 4 subchannels around one fuel pin (quad geometry). SCM sends pin surface temperature to the pin model. The pin model returns linear heat flux at the surface of the pin and pin diameter. The pin diameter is affected by thermal dillation.

## Input files

The input files are the following:

!listing /examples/coupling/thermo_mech/quad/one_pin_problem.i language=cpp

!listing /examples/coupling/thermo_mech/quad/one_pin_problem_sub.i language=cpp




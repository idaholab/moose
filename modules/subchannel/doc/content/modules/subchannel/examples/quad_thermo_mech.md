# Thermo-mechanical coupling of an one pin problem

This is an example case of coupling a subchannel calculation (`SCM`) with a 2D-RZ thermo_mechanical model of a fuel pin.


## Example Description

The example models 4 subchannels around one fuel pin (quadrilateral geometry). `SCM` calculates coolant temperature distribution and an average pin surface temperature `Tpin` that gets transfered to the pin model. The pin model calculates heat conduction within the pin and thermal expansion. Then the pin model returns linear heat flux at the surface of the pin `q_prime` and the updated pin diameter `Dpin` to `SCM`. In order for the updated pin diameter to be considered by the `SCM` model the user must have activated the flag: [!param](/Problem/QuadSubChannel1PhaseProblem/deformation) = true in the problem/SubChannel block of the `SCM` input file.

## Input files

The input files are the following:

`SCM` Model.

!listing /examples/coupling/thermo_mech/quad/one_pin_problem.i language=cpp

Fuel Pin Model.

!listing /examples/coupling/thermo_mech/quad/one_pin_problem_sub.i language=cpp




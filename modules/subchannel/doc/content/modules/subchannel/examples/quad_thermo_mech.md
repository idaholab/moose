# Thermo-mechanical coupling of an one pin problem

This is an example case of coupling a subchannel calculation (`SCM`) with a 2D-RZ thermo_mechanical model of a fuel pin.

## Example Description

!! Intentional comment to provide extra spacing

The example models 4 subchannels around one fuel pin (quadrilateral geometry). `SCM` calculates coolant temperature distribution and an average pin surface temperature `Tpin` that gets transfered to the pin model. The pin model calculates heat conduction within the pin and thermal expansion. Then the pin model returns linear heat flux at the surface of the pin `q_prime` and the updated pin diameter `Dpin` to `SCM`.

Both models are given the same pin power, 100 kW, with the same axial shape. The transfer of `q_prime` to `SCM` preserves the heat rate through the pin surface, computed in the pin model with [SideDiffusiveFluxIntegral](SideDiffusiveFluxIntegral.md): it scales `q_prime` so that the pin power seen by `SCM`, [SCMPinPowerPostprocessor](SCMPinPowerPostprocessor.md), equals that heat rate. `q_prime` is first projected onto the nodes of the pin mesh with [ProjectionAux](ProjectionAux.md), so that the transfer to the subchannel nodes takes nodal values instead of the nearest of several element centroids. Without this, the nearest-location transfer of `q_prime` from the pin model's axial cells to the subchannel nodes loses about 1.4% of the power.

`SCM` does not compute a nonlinear residual, so the fixed-point tolerance of the coupling has no effect, and the number of coupling iterations is set by `fixed_point_min_its` and `fixed_point_max_its`. The pin power in `SCM` is converged after two coupling iterations, and the input uses three.

## Input files

!! Intentional comment to provide extra spacing

The input files are the following:

`SCM` Model.

!listing modules/combined/test/tests/subchannel_thermo_mech/one_pin_problem.i language=moose

Fuel Pin Model.

!listing modules/combined/test/tests/subchannel_thermo_mech/one_pin_problem_sub.i language=moose

## Caveat

!! Intentional comment to provide extra spacing

The SCM input file utilizes the [MultiApp](https://mooseframework.inl.gov/syntax/MultiApps/index.html) functionality and runs the fuel model as a SubApp. The executable to be used is the combined-opt that is compiled with the make file in combined folder of MOOSE. The input files are in `modules/combined/test/tests/subchannel_thermo_mech`.

# Thermo-mechanical coupling of an one pin problem

This is an example case of coupling a subchannel calculation (`SCM`) with a 2D-RZ thermo_mechanical model of a fuel pin.

## Example Description

!! Intentional comment to provide extra spacing

The example models 4 subchannels around one fuel pin (quadrilateral geometry). `SCM` calculates coolant temperature distribution and an average pin surface temperature `Tpin` that gets transfered to the pin model. The pin model calculates heat conduction within the pin and thermal expansion. Then the pin model returns linear heat flux at the surface of the pin `q_prime` and the updated pin diameter `Dpin` to `SCM`.

Only these three fields cross the coupling. The coolant velocity (through the mass flow rate `mdot`), pressure `P`, enthalpy `h` and temperature `T` are solved by `SCM` alone. The pin model sees the coolant only through `Tpin`, which `SCM` computes from the coolant temperature and the pin heat transfer coefficient, and `SCM` sees the pin only through `q_prime` and `Dpin`, which changes the flow area and wetted perimeter of the subchannels.

The input provides two ways of transferring `Tpin` to the pin model:

- By default, a [MultiAppGeneralFieldNearestLocationTransfer](MultiAppGeneralFieldNearestLocationTransfer.md) (transfer `Tpin`) gives each pin surface node the value of the nearest `SCM` axial node, and a [MatchedValueBC](MatchedValueBC.md) imposes it (boundary condition `right`). This works for any number of pins without changes, but the pin surface mesh (100 axial cells) is finer than the `SCM` axial mesh (25 cells), so the imposed temperature is piecewise constant along the pin, with steps of up to 10 K in this example.
- Alternatively, `Tpin` is sampled at the `SCM` axial nodes with a [LineValueSampler](LineValueSampler.md) and sent to the pin model as a profile with a [MultiAppReporterTransfer](MultiAppReporterTransfer.md) (transfer `Tpin_interpolated`). The pin model interpolates it linearly along the pin axis with a [PiecewiseLinearFromVectorPostprocessor](PiecewiseLinearFromVectorPostprocessor.md) and imposes it on the pin surface with a [FunctionDirichletBC](FunctionDirichletBC.md) (boundary condition `right_interpolated`). The pin surface temperature is then continuous and equals `Tpin` at the `SCM` axial nodes. The sampling line is the axis of the single pin, so a case with several pins needs one sampler per pin. `MultiAppReporterTransfer` warns that it does not apply the rotation of the pin model (`beta_rotation`), which does not matter here because the profile depends only on the axial coordinate.

The interpolating variant is selected from the command line:

```
combined-opt -i one_pin_problem.i Transfers/inactive=Tpin sub:BCs/inactive=right
```

The two variants give the same pin power to within $10^{-5}$ W, because the heat rate through the pin surface is set by the heat source and depends little on how the surface temperature varies along the pin.

Both models are given the same pin power, 100 kW, with the same axial shape. The transfer of `q_prime` to `SCM` preserves the heat rate through the pin surface computed in the pin model: it scales `q_prime` so that the pin power seen by `SCM`, [SCMPinPowerPostprocessor](SCMPinPowerPostprocessor.md), equals that heat rate. `q_prime` is first projected onto the nodes of the pin mesh with [ProjectionAux](ProjectionAux.md), so that the transfer to the subchannel nodes takes nodal values instead of the nearest of several element centroids. Without this, the nearest-location transfer of `q_prime` from the pin model's axial cells to the subchannel nodes loses about 1.4% of the power.

The heat rate through the pin surface is computed from the residual of the heat conduction equation rather than from the temperature gradient. The heat conduction and heat source kernels also add their contributions to a tagged vector (`extra_vector_tags`), which [TagVectorAux](TagVectorAux.md) copies into an auxiliary variable. At the nodes of the pin surface, where the temperature is imposed, this residual is the heat flowing through the surface, and [NodalSum](NodalSum.md) adds it up (postprocessor `pin_surface_heat_rate`). The discrete equations balance it with the heat source, so it equals the 100 kW pin power up to the nonlinear solver tolerance on any mesh. The pin power and the coolant enthalpy rise in `SCM`, [SCMTHPowerPostprocessor](SCMTHPowerPostprocessor.md), are then both 100 kW, and the coupling conserves the heat flux and the power.

The integral of the temperature gradient over the pin surface, [SideDiffusiveFluxIntegral](SideDiffusiveFluxIntegral.md) (postprocessor `pin_surface_heat_rate_gradient`), is less accurate. The gradient of the finite element temperature is one order less accurate than the temperature itself, and on the pin mesh of this example the integral exceeds the heat source by 3 W (0.003%). The excess converges to zero at second order with mesh refinement.

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

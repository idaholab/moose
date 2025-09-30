# LinearFVRobinCHTBC

## Description

This object is a boundary condition used in the Navier-Stokes module that can be used for conjugate heat transfer (CHT) problems.
Its role is to enforce a Robin-type boundary condition at an interface where heat is exchanged between solid and fluid regions.
It is similar in behavior to [LinearFVAdvectionDiffusionFunctorRobinBC.md] with the
coefficients in the expression aligned with the thermal-hydraulics fields.
For more information on the design of CHT, click [here](linear_fv_cht.md).

This boundary condition needs an incoming flux which is typically provided by the
SIMPLE-type executioner when conjugate heat transfer is enabled. The
automatically created functors within [SIMPLE.md] are:

- heat_flux_to_solid_* (where * is the interface boudary name)
- heat_flux_to_fluid_* (where * is the interface boudary name)

Furthermore, this boundary condition also requires a surface temperature which
is typically the surface temperature on the other side of the interface.
The interface temperatures are also tracked in fuctors and are created within the
[SIMPLE.md] executioner. The following surface termperatures are created automatically:

- interface_temperature_solid_* (where * is the interface boudary name)
- interface_temperature_fluid_* (where * is the interface boudary name)

!listing modules/navier_stokes/test/tests/finite_volume/ins/cht/conjugate_heat_transfer/cht_rob-rob_top.i block=fluid_solid solid_fluid

!syntax parameters /LinearFVBCs/LinearFVRobinCHTBC

!syntax inputs /LinearFVBCs/LinearFVRobinCHTBC

!syntax children /LinearFVBCs/LinearFVRobinCHTBC

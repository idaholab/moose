# LinearFVDirichletCHTBC

## Description

This object is a boundary condition used in the Navier-Stokes module that can be used for conjugate heat transfer (CHT) problems.
Its role is to enforce a fixed Dirichlet value at an interface where heat is exchanged between solid and fluid regions.
In practice, it is a wrapper around [LinearFVAdvectionDiffusionFunctorDirichletBC.md] with additional content
allowing error checking in CHT applications. For more information on the design of CHT, click [here](linear_fv_cht.md).

!listing modules/navier_stokes/test/tests/finite_volume/ins/cht/conjugate_heat_transfer/cht_neu-dir_top.i block=fluid_solid solid_fluid

!syntax parameters /LinearFVBCs/LinearFVDirichletCHTBC

!syntax inputs /LinearFVBCs/LinearFVDirichletCHTBC

!syntax children /LinearFVBCs/LinearFVDirichletCHTBC

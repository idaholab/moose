# PINSFVFunctorBC

!syntax description /FVBCs/PINSFVFunctorBC

## Overview

This object is very similar to [PCNSFVStrongBC.md]. However,
this object is designed for use in tandem with an
incompressible/weakly-compressible set of objects (see
[PINSFVMomentumAdvection.md]) while [PCNSFVStrongBC.md] is meant for use in
tandem with a fully compressible set of objects (see [PCNSFVKT.md]).

A *seeming* difference between this object and [PCNSFVStrongBC.md] is that variable
boundary values *appear* to be implicit, e.g. we compute the boundary variable
values by calling `operator()` on the variable objects. However, behind the scenes
`operator()` will use any prescribed Dirichlet boundary information if it is
present. If it is not present, the boundary face value will be determined by
extrapolating to the boundary face using the cell centered value and gradient
(the latter is only used if `two_term_boundary_expansion = true` for the
variable in question). As a result of these behind-the-scenes operations, the
functional behavior of `PINSFVFunctorBC` and `PCNSFVStrongBC` are quite similar.

Note that if this object is used in conjunction with
[PINSFVMomentumPressureFlux.md], then `force_boundary_execution` must be set to
`false` in the momentum pressure flux input file block since `PINSFVFunctorBC`
already adds the pressure flux contribution no the boundary. `PINSFVFunctorBC`
should not be used in conjunction with [PINSFVMomentumPressure.md]

!syntax parameters /FVBCs/PINSFVFunctorBC

!syntax inputs /FVBCs/PINSFVFunctorBC

!syntax children /FVBCs/PINSFVFunctorBC

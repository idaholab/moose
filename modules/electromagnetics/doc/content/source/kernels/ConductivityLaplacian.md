# ConductivityLaplacian

!syntax description /Kernels/ConductivityLaplacian

The Jacobian contribution resulting from this residual is also computed using
the MOOSE [automatic_differentiation/index.md] System.

## Example Input File Syntax

!listing contact_conductance_supplied.i block=Kernels/electric_graphite

!syntax parameters /Kernels/ConductivityLaplacian

!syntax inputs /Kernels/ConductivityLaplacian

!syntax children /Kernels/ConductivityLaplacian

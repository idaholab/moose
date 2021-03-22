# CoupledPenaltyInterfaceDiffusion

## Description

This class ensures the continuity of flux of `variable` and `neighbor_var`
across an interface. Additionally, via a penalty factor,
`CoupledPenaltyInterfaceDiffusion` enforces the continuity of the value of
`secondary_coupled_var` and `primary_coupled_var`. The latter two parameters are
optional input file parameters; if they are not specified, then they default to
`neighbor_var` and `variable` respectively. Note that the penalty method (for
sufficiently high penalty parameter) displays optimal convergence rates (p + 1)
while strong enforcement of value continuity results in degradation of the
convergence rate. For a discussion of this phenomena, see
[this moose forum discussion](https://groups.google.com/forum/#!searchin/moose-users/yaqi$20alex%7Csort:date/moose-users/fi68gQRZl9g/n4Q1OzMRBAAJ).

!syntax parameters /InterfaceKernels/CoupledPenaltyInterfaceDiffusion

!syntax inputs /InterfaceKernels/CoupledPenaltyInterfaceDiffusion

!syntax children /InterfaceKernels/CoupledPenaltyInterfaceDiffusion

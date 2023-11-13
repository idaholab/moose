# kEpsilonViscosityAux

!syntax description /AuxKernels/kEpsilonViscosityAux

This is the axiliary kernel used to compute the dynamic turbulence viscosity $\mu_t = \rho C_{\mu} \frac{k^2}{\epsilon}$,
where:

- $\rho$ is the density,
- $C_{\mu} = 0.09$ is a closure parameter,
- $k$ is the turbulent kinetic energy,
- $\epsilon$ is the turbulent kinetic energy dissipation rate.

!syntax parameters /AuxKernels/kEpsilonViscosityAux

!syntax inputs /AuxKernels/kEpsilonViscosityAux

!syntax children /AuxKernels/kEpsilonViscosityAux

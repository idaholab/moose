# INSFVv2fViscosityFunctorMaterial

This is the auxiliary kernel used to compute the dynamic turbulent viscosity for the v2f model.

\begin{equation}
\mu_t = \rho \min \left[ C_{\mu} \frac{k^2}{\epsilon}, C_{\mu 2} \overline{v^2} T_s \right]\,,
\end{equation}

where:

- $\rho$ is the density,
- $C_{\mu} = 0.09$ is a closure parameter,
- $k$ is the turbulent kinetic energy,
- $\epsilon$ is the turbulent kinetic energy dissipation rate,
- $C_{\mu 2} = 0.22$ is a model closure parameter,
- $\overline{v^2}$ is the wall normal Reynolds stresses,
- $T_s$ is the v2f model time scale.

The v2f model time scale is defined as follows:

\begin{equation}
T_s = \max \left( \frac{k}{\epsilon}, 6.0 \sqrt{\frac{\mu}{\rho \epsilon}} \right) \,,
\end{equation}

where:

- $\mu$ is the dynamic molecular viscosity,
- $\rho$ is the density.

!alert note
If the user is not doing enhanced wall treatments,
wall boundary conditions for the turbulent viscosity should be set via
[INSFVTurbulentViscosityWallFunction](INSFVTurbulentViscosityWallFunction.md).

!syntax parameters /AuxKernels/INSFVv2fViscosityFunctorMaterial

!syntax inputs /AuxKernels/INSFVv2fViscosityFunctorMaterial

!syntax children /AuxKernels/INSFVv2fViscosityFunctorMaterial

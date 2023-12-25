# INSFVkEpsilonViscosityFunctorMaterial

This is the material class used to compute the dynamic turbulent viscosity

\begin{equation}
  \mu_t = \rho C_{\mu} \frac{k^2}{\epsilon} \,,
\end{equation}

where:

- $\rho$ is the density,
- $C_{\mu} = 0.09$ is a closure parameter,
- $k$ is the turbulent kinetic energy,
- $\epsilon$ is the turbulent kinetic energy dissipation rate.

!syntax parameters /FunctorMaterials/INSFVkEpsilonViscosityFunctorMaterial

!syntax inputs /FunctorMaterials/INSFVkEpsilonViscosityFunctorMaterial

!syntax children /FunctorMaterials/INSFVkEpsilonViscosityFunctorMaterial

# (AD)RadiativeP1DiffusionCoefficientMaterial

This [functor material](/FunctorMaterials/index.md) computes the effective
P1 radiative heat transfer coefficient as follows:

\begin{equation}
\Gamma = \frac{1}{3 \kappa + \sigma_{s, eff}}
\end{equation}

where

- $\kappa$ is the opacity (SI units $m^{-1}$)
- $\sigma_{s, eff}$ is the P1 effective scattering cross section (SI units $m^{-1}$), e.g., $3 \sigma_{tr}$ with $\sigma_{tr}$ being the transport cross section.

!alert warning
If scattering in the medium can be assumed isotropic,
then $\sigma_{s, eff} \approx 3 \sigma_{s}$ is a good approximation [!citep](incropera2002).

!syntax parameters /FunctorMaterials/RadiativeP1DiffusionCoefficientMaterial

!syntax inputs /FunctorMaterials/RadiativeP1DiffusionCoefficientMaterial

!syntax children /FunctorMaterials/RadiativeP1DiffusionCoefficientMaterial

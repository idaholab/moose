# VectorEMRobinBC

!syntax description /BCs/VectorEMRobinBC

## Overview

!style halign=left
The VectorEMRobinBC object is an implementation of the first-order Robin-style boundary
condition outlined in [!citep](jin-fem) Equation 9.60 and [!citep](jin-computation)
Equation 9.3.51 for vector field variables.

#### General (vector field) form

!style halign=left
The generic Jin condition is, in turn, based on the Sommerfeld radiation condition
for scattered fields. Given that any scattered field can be made up of a
combination of the scattered field and the incident field
($\vec{E} = \vec{E}_{sc} + \vec{E}_{inc}$), then we have

\begin{equation}
  \hat{\mathbf{n}} \times \left( \frac{1}{\mu_r} \nabla \times \vec{E} \right) + \frac{j k_0}{\eta_r} \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E} \right) = \hat{\mathbf{n}} \times (\nabla \times \vec{E}_{inc}) + \frac{j k_0}{\eta_r} \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E}_{inc} \right)
\end{equation}

where

- $\vec{E}$ is the total (i.e., the solution) electric field vector,
- $\vec{E}_{inc}$ is the incoming electric field vector,
- $\mu_r$ is the relative magnetic permeability,
- $j = \sqrt{-1}$,
- $k_0$ is the wave number ($2 \pi / \lambda$ where $\lambda$ is the wavelength),
- $\eta_r$ is a radiation condition parameter ($\eta_r = 1$ if the condition is applied in free space), and
- $\hat{\mathbf{n}}$ is the boundary normal vector.

#### Implemented form

!style halign=left
In VectorEMRobinBC, this condition is slightly generalized. The ratio of $k_0 / \eta_r$
is generalized to a coefficient function $\beta$, and so the implemented form is

\begin{equation}
  \hat{\mathbf{n}} \times \left( \frac{1}{\mu_r} \nabla \times \vec{E} \right) + j\beta(\mathbf{r}) \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E} \right) = \hat{\mathbf{n}} \times (\nabla \times \vec{E}_{inc}) + j\beta(\mathbf{r}) \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E}_{inc} \right)
\end{equation}

where

- $\beta(\mathbf{r})$ is a function containing the condition coefficients.

Note that $\beta$ generally is just the wavenumber $k$, since this condition is
generally applied in free space, but any function can be applied through the
[!param](/BCs/VectorEMRobinBC/beta) parameter.

#### Usage notes

!style halign=left
It is important to note that when used as an absorber (strictly absorbing means
[!param](/BCs/VectorEMRobinBC/mode) is set to `absorbing` and zero incoming wave 
but a port also has an absorbing component), care must be taken in setting the 
shape of the truncation boundary as well as the distance from the scattering object. 
Boundaries as close as $0.3 \lambda$ away from the object was shown in [!citep](jin-fem), 
and several wavelengths were used in the [DipoleAntenna.md] for VectorEMRobinBC. Also, 
this boundary condition is best applied on spherical boundaries (as a result of its
origin from the Sommerfeld condition, which was derived for spherical boundaries).
Of course, it is also valid on any non-spherical smooth surface with a trade-off
in accuracy.

## Example Input File Syntax

### As a Port

!listing portbc_waves.i block=BCs/sides_real

### As an Absorber

!listing dipole.i block=BCs/radiation_condition_real

!syntax parameters /BCs/VectorEMRobinBC

!syntax inputs /BCs/VectorEMRobinBC

!syntax children /BCs/VectorEMRobinBC

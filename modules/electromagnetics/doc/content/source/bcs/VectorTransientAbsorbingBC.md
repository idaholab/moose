# VectorTransientAbsorbingBC

!syntax description /BCs/VectorTransientAbsorbingBC

## Overview

!style halign=left
The VectorTransientAbsorbingBC implements the first-order absorbing boundary
condition mentioned in [!citep](jin-fem) Equation 12.83 for vector variables.
This condition is given by

\begin{equation}
  \hat{\mathbf{n}} \times \left[ \frac{1}{\mu_0} \nabla \times \vec{E}(\mathbf{r},t)\right] + Y_0 \: \hat{\mathbf{n}} \times \left[ \hat{\mathbf{n}} \times \frac{\partial \vec{E}(\mathbf{r}, t)}{\partial t}\right] = 0
\end{equation}

where

- $\vec{E}$ is the electric field vector with complex components,
- $\mu_0$ is the vacuum magnetic permeability,
- $Y_0$ is the intrinsic admittance of the infinite medium, and
- $\hat{\mathbf{n}}$ is the boundary normal vector.

The intrinsic admittance of the infinite medium is set to a default of free space, which is defined as

\begin{equation}
  Y_0 = \sqrt{\frac{\epsilon_0}{\mu_0}} = \frac{1}{\mu_0 c}
\end{equation}

where

- $\epsilon_0$ is the vacuum electric permittivity,
- $\mu_0$ is the vacuum magnetic permeability, and
- $c$ is the speed of light.

The admittance may also be complex, with its real and imaginary components set by
[!param](/BCs/VectorTransientAbsorbingBC/admittance_real) and
[!param](/BCs/VectorTransientAbsorbingBC/admittance_imag). The imaginary component defaults to zero.

## Example Input File Syntax

!listing dipole_transient.i block=BCs/radiation_condition_real

!syntax parameters /BCs/VectorTransientAbsorbingBC

!syntax inputs /BCs/VectorTransientAbsorbingBC

!syntax children /BCs/VectorTransientAbsorbingBC

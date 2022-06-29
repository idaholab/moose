# EMRobinBC

!syntax description /BCs/EMRobinBC

## Overview

!style halign=left
The EMRobinBC object is an implementation of the first-order Robin-style boundary
condition outlined in [!citep](jin-fem) Equation 9.60 and [!citep](jin-computation)
Equation 9.3.51 for scalar field variables.

#### General (vector field) form

!style halign=left
The generic condition from from [!citep](jin-fem) is, in turn, based on the Sommerfeld
radiation condition for scattered fields. Given that any scattered field can be made up of a
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

#### Scalar field form (for EMRobinBC)

!style halign=left
In EMRobinBC, this condition is simplified for use with scalar field variables, such
as those solved for in plane-wave problems where only a component of the vector
field is being solved for. In such a problem, it is assumed that $E \sim e^{jkx}$.
The choice of $x$ as the spatial variable here is arbitrary; the directionality
of the plane wave determines the spatial variable of interest. The implemented
condition in this object is then

\begin{equation}
  \frac{\partial E_x}{\partial x} + jkE_x = j2kF(x)e^{jkx}
\end{equation}

where

- $E_x$ is the scalar field component value,
- $k$ is the wavenumber, and
- $F(x)$ is a function representing the amplitude profile of the incoming wave.

Note that in EMRobinBC, $k$ could be set via the [!param](/BCs/EMRobinBC/coeff_real) and [!param](/BCs/EMRobinBC/coeff_imag)
parameters for a constant wavenumber, or [!param](/BCs/EMRobinBC/func_real) and [!param](/BCs/EMRobinBC/func_imag) for a
property that varies in space or time. The incoming profile is set via
[!param](/BCs/EMRobinBC/profile_func_real) and [!param](/BCs/EMRobinBC/profile_func_imag).


#### Usage notes

!style halign=left
It is important to note that when used as an absorber (strictly absorbing means that `mode = absorbing`
with zero incoming wave but a port also has an absorbing component for any reflections), care must be
taken in setting the shape of the truncation boundary as well as the distance
from the scattering object. Boundaries as close as $0.3 \lambda$ away from the
object was shown in [!citep](jin-fem), and several wavelengths were used in the
[DipoleAntenna.md] for [VectorEMRobinBC.md]. This boundary condition is also best
applied on spherical boundaries (as a result of its origin from the Sommerfeld
condition, which was derived for spherical boundaries). Of course, it is also
valid on any non-spherical smooth surface with a trade-off in accuracy.

## Example Input File Syntax

### As a Port

!listing waveguide2D_test.i block=BCs/port_real

### As an Absorber

!listing waveguide2D_test.i block=BCs/exit_real

!syntax parameters /BCs/EMRobinBC

!syntax inputs /BCs/EMRobinBC

!syntax children /BCs/EMRobinBC

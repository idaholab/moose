# ComplexHeating

!syntax description /Kernels/ComplexHeating

## Overview

!style halign=left
The ComplexHeating object implements a heating term imparted to the medium based on the conduction current. The term is define as:

\begin{equation}
  -0.5 * Re \left(\sigma \; \vec{E} \cdot \vec{E}^{*} \right) * \text{Scaling}
\end{equation}

where

- $\sigma$ is the conductivity of the medium, 
- $\vec{E}$ is the electric field
- $\vec{E}^{*}$ is the complex conjugate of the electric field,
- and $\text{Scaling}$ is a scaling factor (usually used to convert the units of the heating term).


## Example Input File Syntax

!listing vector_helmholtz/microwave_heating.i block=Kernels/microwave_heating

!syntax parameters /Kernels/ComplexHeating

!syntax inputs /Kernels/ComplexHeating

!syntax children /Kernels/ComplexHeating

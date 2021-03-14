# DielectricBC

!syntax description /BCs/DielectricBC

## Overview

!style halign=left
The DielectricBC object is a first attempt to define an electric field field
boundary condition on a dielectric surface, adapted from the interface condition
in [!citep](griffiths-intro) Equation 4.26. Assuming that there is no free charge
build-up on the boundary, this interface condition is

\begin{equation}
  \epsilon_1 \vec{E}_1 \cdot \hat{\mathbf{n}} = \epsilon_2 \vec{E}_2 \cdot \hat{\mathbf{n}}
\end{equation}

where

- $\epsilon_1$ is the electric permittivity on the primary side of the interface,
- $\epsilon_2$ is the electric permittivity on the secondary side of the interface,
- $\vec{E}_1$ is the electric field on the primary side of the interface,
- $\vec{E}_2$ is the electric field on the secondary side of the interface. and
- $\hat{\mathbf{n}}$ is the interface unit normal vector.

In adapting this to an integrated boundary condition, the gradient was taken on
both sides (and the values for permittivity presumed to be constants), and the
permittivities were gathered to the right-hand-side of the condition in a ratio.
Finally, $\vec{E}_2 \rightarrow \vec{E}_1$ so that the discontinuity in the gradient
values at the interface coming from the permittivity ratio could be accumulated
in the residual contribution at the boundary for the solution variable.

\begin{equation}
  \nabla \vec{E}_1 \cdot \hat{\mathbf{n}} = \frac{\epsilon_2}{\epsilon_1} \nabla \vec{E}_1 \cdot \hat{\mathbf{n}}
\end{equation}

## Example Input File Syntax

!alert warning title=This is not currently tested
The DielectricBC object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /BCs/DielectricBC

!syntax inputs /BCs/DielectricBC

!syntax children /BCs/DielectricBC

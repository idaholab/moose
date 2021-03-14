# CoupledGradBC

!syntax description /BCs/CoupledGradBC

## Overview

The CoupledGradBC object imposes the boundary condition

\begin{equation}
  \nabla u \cdot \hat{\mathbf{n}} = s C f(\mathbf{r}) \nabla v \cdot \hat{\mathbf{n}}
\end{equation}

where

- $u$ is the solution variable,
- $v$ is the coupled variable,
- $s$ is the coefficient used to set the sign of the BC (1.0, -1.0, positive default),
- $C$ is a constant coefficient,
- $f(\mathbf{r})$ is a function coefficient, and
- $\hat{\mathbf{n}}$ is the unit normal at the boundary.

## Example Input File Syntax

!alert warning title=This is not currently tested
The CoupledGradBC object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /BCs/CoupledGradBC

!syntax inputs /BCs/CoupledGradBC

!syntax children /BCs/CoupledGradBC

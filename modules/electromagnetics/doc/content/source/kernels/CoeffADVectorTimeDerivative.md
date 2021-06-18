# CoeffADVectorTimeDerivative

!syntax description /Kernels/CoeffADVectorTimeDerivative

## Overview

!style halign=left
The CoeffADVectorTimeDerivative object computes a time derivative PDE term for
use with vector variables. Specifically, the weak form inner product term
calculated in this object is

\begin{equation}
  \left(\vec{\psi}_i \; , \; a(\mathbf{r}, t) \frac{\partial \vec{u}}{\partial t}\right)
\end{equation}

where

- $\vec{\psi}_i$ is a vector-valued test function,
- $\vec{u}$ is the solution vector variable, and
- $a(\mathbf{r}, t)$ is a coefficient function.

The Jacobian contribution resulting from this residual is also computed using
the MOOSE [automatic_differentiation/index.md] System.

## Example Input File Syntax

!alert warning title=This is not currently tested
The CoeffADVectorTimeDerivative object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /Kernels/CoeffADVectorTimeDerivative

!syntax inputs /Kernels/CoeffADVectorTimeDerivative

!syntax children /Kernels/CoeffADVectorTimeDerivative

# CoupledCoeffTimeDerivative

!syntax description /Kernels/CoupledCoeffTimeDerivative

## Overview

!style halign=left
The CoupledCoeffTimeDerivative object computes a coupled time derivative PDE term for
use with scalar variables. Specifically, the weak form inner product term
calculated in this object is

\begin{equation}
  \left(\psi_i \; , \; a \frac{\partial v}{\partial t}\right)
\end{equation}

where

- $\psi_i$ is the scalar-valued test function,
- $v$ is the coupled scalar variable, and
- $a$ is a constant coefficient.

## Example Input File Syntax

!alert warning title=This is not currently tested
The CoupledCoeffTimeDerivative object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /Kernels/CoupledCoeffTimeDerivative

!syntax inputs /Kernels/CoupledCoeffTimeDerivative

!syntax children /Kernels/CoupledCoeffTimeDerivative

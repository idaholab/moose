# HelmholtzTestFunc

!syntax description /Functions/HelmholtzTestFunc

## Overview

!style halign=left
This function object calculates the complex analytical solution used in a
convergence study for the implemented Helmholtz wave equation in ELK.
Specifically, the following PDE for a 1D domain extending from $x = [0, L]$:

\begin{equation}
    k \frac{\text{d}^2 u(x)}{\text{d} x^2} + c(x)^2 u(x) = 0
\end{equation}

where $k$ is a complex constant coefficient and $c$ is a complex function
coefficient. Boundary conditions are defined by

\begin{equation}
  u(0) = a \\
  u(L) = b
\end{equation}

where $a$ and $b$ are complex constants. The analytic solution for this case is

\begin{equation}
u(x) = u(0) * \cos\left(\frac{c(x)}{\sqrt{k}} x \right) + \frac{\left(u(L) - u(0) \cos\left( \frac{c(L)}{\sqrt{k}} L \right) \right)}{\sin\left(\frac{c(L)}{\sqrt{k}} L\right)} \sin\left( \frac{c(x)}{\sqrt{k}} x \right)
\end{equation}

## Example Input File Syntax

!alert warning title=This is not currently tested
The HelmholtzTestFunc object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /Functions/HelmholtzTestFunc

!syntax inputs /Functions/HelmholtzTestFunc

!syntax children /Functions/HelmholtzTestFunc

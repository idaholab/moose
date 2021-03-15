# CoupledCurlCurlField

!syntax description /Kernels/CoupledCurlCurlField

## Overview

!style halign=left
The CoupledCurlCurlField object computes the residual contribution for the term

\begin{equation}
  s \nabla \times \nabla \times \vec{E}_{coupled}
\end{equation}

where

- $\vec{E}_{coupled}$ is a coupled electric field vector variable, and
- $s$ is a constant coefficient corresponding to the sign of the term in the weak form (default = 1.0)

## Example Input File Syntax

!alert warning title=This is not currently tested
The CoupledCurlCurlField object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /Kernels/CoupledCurlCurlField

!syntax inputs /Kernels/CoupledCurlCurlField

!syntax children /Kernels/CoupledCurlCurlField

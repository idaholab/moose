# CurlCurlField

!syntax description /Kernels/CurlCurlField

## Overview

!style halign=left
The CurlCurlField object implements the following PDE term for vector variables:

\begin{equation}
  \nabla \times \left( a \; \nabla \times \vec{E} \right)
\end{equation}

where

- $a$ is a constant coefficient (default = 1.0), and
- $\vec{E}$ is the electric field vector variable.

## Example Input File Syntax

!listing vector_kernels.i block=Kernels/curl_curl

!syntax parameters /Kernels/CurlCurlField

!syntax inputs /Kernels/CurlCurlField

!syntax children /Kernels/CurlCurlField

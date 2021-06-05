# CurlCurlField

!syntax description /Kernels/CurlCurlField

## Overview

!style halign=left
The CurlCurlField object implements the following PDE term for vector variables:

\begin{equation}
  \nabla \times \left( a \; \nabla \times \vec{u} \right)
\end{equation}

where

- $a$ is a constant coefficient (default = 1.0), and
- $\vec{u}$ is the solution field vector variable.

## Example Input File Syntax

!listing vector_kernels.i block=Kernels/curl_curl

!syntax parameters /Kernels/CurlCurlField

!syntax inputs /Kernels/CurlCurlField

!syntax children /Kernels/CurlCurlField

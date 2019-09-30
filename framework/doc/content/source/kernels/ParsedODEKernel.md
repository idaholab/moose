# ParsedODEKernel

## Description

`ParsedODEKernel` implements an ODE term using a parsed function $f$ specified
in the input file
\begin{equation}
(\psi_i, f(\vec v, \vec p)).
\end{equation}
The function $f$ can depend on a list $\vec v$ of coupled variables (`args`) and
a list $\vec p$ of postprocessor values (`postprocessors`).

!listing examples/ex18_scalar_kernel/ex18_parsed.i block=ScalarKernels

!syntax parameters /ScalarKernels/ParsedODEKernel

!syntax inputs /ScalarKernels/ParsedODEKernel

!syntax children /ScalarKernels/ParsedODEKernel

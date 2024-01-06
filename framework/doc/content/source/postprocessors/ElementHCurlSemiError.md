# ElementHCurlSemiError

!syntax description /Postprocessors/ElementHCurlSemiError

## Overview

The H(curl)-semierror between a vector-valued solution $\vec{u}$ and a known
vector-valued function $\vec{f}$ is written

\begin{equation}
    \left( \int_{\Omega} || \nabla \times \vec{u}  - \nabla \times \vec{f} ||^2 \,\text{d}x \right) ^{1/2}
\end{equation}

where $||\cdot||$ denotes the l2-norm in $\mathbb{R}^d$.

The nonlinear vector variable specified for $\vec{u}$ must be of the
`NEDELEC_ONE` family, as MOOSE currently does not compute the curl for
any other finite element family.

$\vec{f}$ must be a [Function](syntax/Functions/index.md) object defining both
the `vectorValue` and `curl` methods, e.g. [MooseParsedVectorFunction.md].

## Example Input File Syntax

!listing vector_kernel.i block=Postprocessors/HCurlSemiError

!syntax parameters /Postprocessors/ElementHCurlSemiError

!syntax inputs /Postprocessors/ElementHCurlSemiError

!syntax children /Postprocessors/ElementHCurlSemiError
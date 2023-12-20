# ElementVectorL2Error

!syntax description /Postprocessors/ElementVectorL2Error

## Overview

The L2-error between a vector-valued solution $\vec{u}$ and a known
vector-valued function $\vec{f}$ is written

\begin{equation}
    \left( \int_{\Omega} || \vec{u}  - \vec{f} ||^2 \,\text{d}x \right) ^{1/2}
\end{equation}

where $||\cdot||$ denotes the l2-norm in $\mathbb{R}^d$.

The user can specify $\vec{u}$ with either a single nonlinear vector variable,
or with up to three nonlinear scalar variables simultaneously, which would each
typically represent the x, y, z components of the vector variable.

Likewise, the user can also specify $\vec{f}$ with either a single
[Function](syntax/Functions/index.md) object defining the `vectorValue` method,
e.g. [MooseParsedVectorFunction.md], or up to three such objects defining
the `value` method, e.g. [MooseParsedFunction.md].

## Example Input File Syntax

With a single nonlinear vector variable and a single vector-valued function:

!listing coupled_electrostatics.i block=Postprocessors/L2Error

With component-wise specifications for both the nonlinear variable and the function:

!listing element_vec_l2_error.i block=Postprocessors/integral

!syntax parameters /Postprocessors/ElementVectorL2Error

!syntax inputs /Postprocessors/ElementVectorL2Error

!syntax children /Postprocessors/ElementVectorL2Error
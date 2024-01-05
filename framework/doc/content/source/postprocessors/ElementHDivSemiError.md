# ElementHDivSemiError

!syntax description /Postprocessors/ElementHDivSemiError

## Overview

The H(div)-semierror between a vector-valued solution $\vec{u}$ and a known
vector-valued function $\vec{f}$ is written

\begin{equation}
    \left( \int_{\Omega} | \nabla \cdot \vec{u}  - \nabla \cdot \vec{f} |^2 \,\text{d}x \right) ^{1/2}
\end{equation}

where $|\cdot|$ denotes the absolute value.

$\vec{f}$ must be a [Function](syntax/Functions/index.md) object defining both
the `vectorValue` and `div` methods, e.g. [MooseParsedVectorFunction.md].

## Example Input File Syntax

!listing coupled_electrostatics.i block=Postprocessors/HDivSemiError

!syntax parameters /Postprocessors/ElementHDivSemiError

!syntax inputs /Postprocessors/ElementHDivSemiError

!syntax children /Postprocessors/ElementHDivSemiError
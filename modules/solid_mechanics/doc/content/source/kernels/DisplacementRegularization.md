# DisplacementRegularization

!syntax description /Kernels/DisplacementRegularization

## Description

`DisplacementRegularization` adds a regularization term to one displacement component in
the solid mechanics displacement equation. For vector-valued displacement variables, add
one scalar kernel per component.

!include modules/solid_mechanics/common/supplementalDisplacementRegularization.md

For displacement component $u$, test function $v$, coefficient $k$, and mesh dimension
$d$, the supported options are

\begin{equation}
R_i^{\mathrm{HuHu}} = \int_\Omega k \, u_{,jk} v_{i,jk} \, d\Omega ,
\end{equation}

\begin{equation}
R_i^{\mathrm{LuLu}} = \int_\Omega k \, \Delta u \Delta v_i \, d\Omega ,
\end{equation}

and

\begin{equation}
R_i^{\mathrm{HuHu-LuLu}} =
\int_\Omega k \left( u_{,jk} v_{i,jk} - c_L \Delta u \Delta v_i \right) d\Omega .
\end{equation}

Here $u_{,jk}$ and $v_{i,jk}$ are Hessian components, $\Delta u$ and
$\Delta v_i$ are Laplacians, and $c_L$ is [!param](/Kernels/DisplacementRegularization/lulu_factor).
The names HuHu and LuLu follow terminology used for third-medium contact regularization
[!cite](frederiksen2025improved,bluhm2021internal), but the kernel only contributes the
regularization term and is not tied to contact-specific infrastructure.

For [!param](/Kernels/DisplacementRegularization/regularization) set to `huhu_lulu`,
[!param](/Kernels/DisplacementRegularization/lulu_factor) defaults to $1 / d$. Values larger
than $1 / d$ are accepted with a warning because they may cause negative strain-energy
contributions.

For vector-valued mechanics fields, add one scalar regularization kernel per displacement
component:

```text
[Kernels]
  [regularize_x]
    type = DisplacementRegularization
    variable = disp_x
    regularization = huhu_lulu
    coefficient = 1
  []
  [regularize_y]
    type = DisplacementRegularization
    variable = disp_y
    regularization = huhu_lulu
    coefficient = 1
  []
[]
```

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/displacement_regularization/displacement_regularization_default_factor.i block=Kernels

!syntax parameters /Kernels/DisplacementRegularization

!syntax inputs /Kernels/DisplacementRegularization

!syntax children /Kernels/DisplacementRegularization

!bibtex bibliography

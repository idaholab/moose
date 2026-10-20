# SecondDerivativeRegularization / ADSecondDerivativeRegularization

!syntax description /Kernels/SecondDerivativeRegularization

## Description

`SecondDerivativeRegularization` adds a regularization term based on second derivatives of a
scalar nonlinear variable. For vector-valued fields, add one scalar kernel per component.

!alert warning title=Second-derivative FE data required
This kernel evaluates second derivatives of the trial and test functions. Use an FE
family and order for which libMesh provides meaningful second derivatives on the selected
elements; the example inputs use `family = LAGRANGE` and `order = SECOND`. Global $C^1$
continuity is not required by this kernel. On elements whose shape functions are linear,
such as `TRI3` and `TET4`, the second derivatives vanish and the kernel adds nothing.
Bilinear elements such as `QUAD4` keep the mixed derivative, so `huhu` still acts on them,
while `lulu` does not.

The included tests run on $C^0$ quadratic Lagrange elements. This is non-conforming for a
standalone biharmonic solve, but the kernel is used as a regularization contribution rather
than as a conforming biharmonic discretization. The cantilever test adds the regularization
to both displacement components of a plane-strain elastic beam under a tip shear load and
shows that it stiffens the bending response by an amount that depends on the regularization
type.

In one dimension HuHu and LuLu are identical, so `huhu_lulu` with the default LuLu factor
of 1 adds nothing. Use `huhu` or `lulu` for one-dimensional problems.

For nonlinear variable $u$, test function $v$, coefficient $k$, and mesh dimension $d$,
the supported options are

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
$\Delta v_i$ are Laplacians, and $c_L$ is [!param](/Kernels/SecondDerivativeRegularization/lulu_factor).
The names HuHu and LuLu follow terminology used for third-medium contact regularization
[!cite](frederiksen2025improved,bluhm2021internal), but the kernel only contributes the
regularization term and is not tied to contact-specific infrastructure.

For [!param](/Kernels/SecondDerivativeRegularization/regularization) set to `huhu_lulu`,
[!param](/Kernels/SecondDerivativeRegularization/lulu_factor) defaults to $1 / d$. Values larger
than $1 / d$ are accepted with a warning because they may cause negative strain-energy
contributions.

For vector-valued mechanics fields, add one scalar regularization kernel per displacement
component:

```text
[Kernels]
  [regularize_x]
    type = SecondDerivativeRegularization
    variable = disp_x
    regularization = huhu_lulu
    coefficient = 1
  []
  [regularize_y]
    type = SecondDerivativeRegularization
    variable = disp_y
    regularization = huhu_lulu
    coefficient = 1
  []
[]
```

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/second_derivative_regularization/cantilever.i block=Kernels

!syntax parameters /Kernels/SecondDerivativeRegularization

!syntax inputs /Kernels/SecondDerivativeRegularization

!syntax children /Kernels/SecondDerivativeRegularization

!bibtex bibliography

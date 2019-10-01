# UnobstructedPlanarViewFactor

## Description

`UnobstructedPlanarViewFactor.md` computes the view factors between `n` planar sides in radiative heat exchange.
These sides need to be such that they do not obstruct each other. This is in particular true if the sides fully enclose
a convex volume. This is the intended purpose of this UserObject.

View factors $F_{i,j}$ from side $i$ to side $j$ are computed via a double loop over side elements and the quadrature points
defined on them. The formula:

\begin{equation}
  F_{1,2} = \frac{1}{A_1 \pi} \int_{A_1} \frac{\cos \beta_1 \cos \beta_2}{r^2}  \int_{A_2} dA_1 dA_2,
\end{equation}

where $r$ is the distance between two points on the surfaces $A_1$ and $A_2$, is numerically evaluated.
View factors should satisfy:

\begin{equation}
  \sum\limits_{j=1}^n F_{i,j} = 1,~i=1,..,n.
\end{equation}
This can be checked by setting the parameter `view_factor_tol` and it can be enforced via normalization by setting the
parameter `normalize_view_factor`.

It is stressed that this UserObject may give wrong results if obstruction is present

## Example Input syntax

!listing modules/heat_conduction/test/tests/view_factors/view_factor_cube.i
block=UserObjects

!syntax parameters /UserObjects/UnobstructedPlanarViewFactor

!syntax inputs /UserObjects/UnobstructedPlanarViewFactor

!syntax children /UserObjects/UnobstructedPlanarViewFactor

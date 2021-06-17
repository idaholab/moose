# UnobstructedPlanarViewFactor

## Description

`UnobstructedPlanarViewFactor` computes the view factors between `n` planar sides in radiative heat exchange.
These sides need to be such that they do not obstruct each other. This is in particular true if the sides fully enclose
a convex volume. This is the intended purpose of this UserObject.

View factors $F_{i,j}$ from side $i$ to side $j$ are computed via a double loop over side elements and the quadrature points
defined on them. View factors are computed by numerically evaluating:

\begin{equation}
  F_{1,2} = \frac{1}{A_1 \pi} \int_{A_1} \int_{A_2}  \frac{\cos \beta_1 \cos \beta_2}{r^2}  dA_1 dA_2,
\end{equation}

where $r$ is the distance between two points on the surfaces $A_1$ and $A_2$ and $\beta_1$ and $\beta_2$ are the angles that the line connecting these two points make with the normals at surface one and two, respectively.

In two-dimensional geometries, a different formula is evaluated. It is derived from the original
formula by considering a geometry that is extruded from $-\infty$ to $\infty$ along the $z$-axis.
We denote by $r_0$ the distance between two points on surface one and two projected onto the plane orthogonal to the $z$-axis. The line projected on this plane makes angles $\beta_{1,0}$ and $\beta_{2,0}$ with the normals at surfaces one and two, respectively. Note that the normals have no component into the $z$-direction. The following relationships hold:

\begin{equation}
  \begin{aligned}
    r^2 &= r_0^2 + (z_1 - z_2)^2 \\
    \cos \beta_{1,0} &= \cos \beta_{1}  \frac{r_0}{\sqrt{r_0^2 + (z_1 - z_2)^2}}\\
    \cos \beta_{2,0} &= \cos \beta_{2}  \frac{r_0}{\sqrt{r_0^2 + (z_1 - z_2)^2}}\\
    A_1 &= \Delta z L_1 \\
    dA_1&= dz_1 dl_1 \\
    dA_2&= dz_2 dl_2.
  \end{aligned}
\end{equation}

The view factor is then given by:

\begin{equation}
  F_{1,2} = \lim\limits_{\Delta z \rightarrow \infty} \frac{1}{L_1 \Delta z \pi} \int_{L1} dl_1
  \int_{L2} dl_2
  \frac{\cos \beta_{1,0} \cos \beta_{2,0} r_0^2}{\pi}
  \left [ \int_{- \Delta z / 2}^{\Delta z / 2} dz_1
  \int_{- \Delta z / 2}^{\Delta z / 2} dz_2 \frac{1}{\left( r_0^2 + (z_1 - z_2)^2 \right)^2} \right]
\end{equation}

The integral in brackets evaluates to:

\begin{equation}
\int_{- \Delta z / 2}^{\Delta z / 2} dz_1
\int_{- \Delta z / 2}^{\Delta z / 2} dz_2 \frac{1}{\left( r_0^2 + (z_1 - z_2)^2 \right)^2}
 = \frac{\Delta z \pi}{2 r_0^3}.
\end{equation}

The view factors in two-dimensional geometry are consequently given by:

\begin{equation}
  F_{1,2} = \frac{1}{2 L_1} \int_{L1}  \int_{L2}
  \frac{\cos \beta_{1,0} \cos \beta_{2,0}}{r_0} dl_1 dl_2
\end{equation}

View factors should satisfy:

\begin{equation}
  \sum\limits_{j=1}^n F_{i,j} = 1,~i=1,..,n.
\end{equation}
This can be checked by setting the parameter `view_factor_tol` and it can be enforced via normalization by setting the
parameter `normalize_view_factor`.

It is stressed that this UserObject may give wrong results if obstruction is present

## Example Input syntax

!listing modules/heat_conduction/test/tests/view_factors/view_factor_2d.i
block=UserObjects

!syntax parameters /UserObjects/UnobstructedPlanarViewFactor

!syntax inputs /UserObjects/UnobstructedPlanarViewFactor

!syntax children /UserObjects/UnobstructedPlanarViewFactor

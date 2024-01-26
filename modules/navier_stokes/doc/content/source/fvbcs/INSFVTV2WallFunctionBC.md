# INSFVTV2WallFunctionBC

Implements wall boundary conditions for $\overline{v^2}$, i.e., $\overline{v^2}_w$, the wall-normal Reynolds stresses.

A different formulation is used for each of the layers of the boundary layer (sublaminar, buffer, and logarithmic).

The layers are defined by $y^+ = \frac{\rho y_p u^*}{\mu}$,
where $\rho$ is the density, $y_p$ is the distance from the center of the next-to-wall cell to the wall,
$u^*$ is the friction velocity, and $\mu$ is the molecular dynamic viscosity.

The friction velocity is defined as $u^* = \sqrt{\sqrt{C_{\mu}} k}$,
where $C_{\mu} = 0.09$ is a turbulent closure parameter, and $k$ is the turbulent kinetic energy.

## Sublaminar layer:

Defined for $y^+ \leq 5$, the following boundary condition is imposed:

\begin{equation}
\overline{v^2}_w^{lam} = C_{v2} {y^+}^4 {u^*}^2 \,,
\end{equation}

where:

- $C_{v2} = 0.193$ is a closure parameter for the distribution of wall-normal stresses for the sublaminar layer.

## Log-layer:

Defined for $y^+ \geq 30$, the following boundary condition is imposed:

\begin{equation}
\overline{v^2}_w^{log} = \left[\frac{C_{v2}}{\kappa} \ln(y^+) + B_{v2} \right] {u^*}^2 \,,
\end{equation}

where:

- $\kappa = 0.4187$ is the Von Karman constant,
- $B_{v2} = -0.94$ is a closure parameter for the distribution of wall-normal stresses for the logarithmic layer.

## Buffer Layer:

Defined for $y^+ \in (0,30)$, the following boundary condition is imposed:

\begin{equation}
\overline{v^2}_w^{buf} =  \frac{y^+ - 5}{25} (\overline{v^2}_w^{log} - \overline{v^2}_w^{lam}) + \overline{v^2}_w^{lam}\,,
\end{equation}

!alert note
Although not recommended, the user may omit this wall function and set the wall
value to `0` when doing enhanced wall treatment.

!syntax parameters /FVBCs/INSFVTV2WallFunctionBC

!syntax inputs /FVBCs/INSFVTV2WallFunctionBC

!syntax children /FVBCs/INSFVTV2WallFunctionBC

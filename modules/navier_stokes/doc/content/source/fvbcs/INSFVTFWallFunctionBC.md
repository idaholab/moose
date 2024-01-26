# INSFVTFWallFunctionBC

Implements wall boundary conditions for $f$, i.e., $f_w$, the v2f elliptic function.

A different formulation is used for each of the layers of the boundary layer (sublaminar, buffer, and logarithmic).

The layers are defined by $y^+ = \frac{\rho y_p u^*}{\mu}$,
where $\rho$ is the density, $y_p$ is the distance from the center of the next-to-wall cell to the wall,
$u^*$ is the friction velocity, and $\mu$ is the molecular dynamic viscosity.

The friction velocity is defined as $u^* = \sqrt{\sqrt{C_{\mu}} k}$,
where $C_{\mu} = 0.09$ is a turbulent closure parameter, and $k$ is the turbulent kinetic energy.

## Sublaminar layer:

Defined for $y^+ \leq 5$, the following boundary condition is imposed:

\begin{equation}
f_w^{lam} = 0 \,,
\end{equation}

## Log-layer:

Defined for $y^+ \geq 30$, the following boundary condition is imposed:

\begin{equation}
f_w^{log} = \frac{n \overline{v^2} \epsilon}{k^2 {u^*}^2} \,,
\end{equation}

where:

- $n = 6$ is a model parameter,
- $\overline{v^2}$ is the wall-normal Reynolds stresses,
- $k$ is the turbulent kinetic energy,
- $\epsilon$ is the dissipation rate of turbulent kinetic energy.

## Buffer Layer:

Defined for $y^+ \in (0,30)$, the following boundary condition is imposed:

\begin{equation}
\overline{v^2}_w^{buf} =  \frac{y^+ - 5}{25} \overline{v^2}_w^{log}\,,
\end{equation}

!alert note
The user may omit this wall function and set the wall value to `0` when doing enhanced wall treatment.

!syntax parameters /FVBCs/INSFVTFWallFunctionBC

!syntax inputs /FVBCs/INSFVTFWallFunctionBC

!syntax children /FVBCs/INSFVTFWallFunctionBC

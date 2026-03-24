# LinearFVTurbulentTemperatureWallFunctionBC

Implements a wall function boundary condition for the effective thermal
conductivity at the wall for the linear finite volume discretization.

This boundary condition computes an effective wall thermal conductivity
used to represent turbulent heat transfer near solid boundaries without
fully resolving the thermal boundary layer. As in
[LinearFVTurbulentViscosityWallFunctionBC.md], the near-wall behavior is
parameterized in terms of the non-dimensional wall distance $y^+$ and the
friction velocity $u_{\tau}$, using the selected
[!param](/LinearFVBCs/LinearFVTemperatureWallFunctionBC/wall_treatment).

The wall heat flux is modeled through an effective conductivity,
$k_{eff,w}$, such that

\begin{equation}
    q_w = \frac{k_{eff,w}}{y_p} \left( T_p - T_w \right)\,,
\end{equation}

where:

- $q_w$ is the wall heat flux,
- $k_{t,w}$ is the effective wall thermal conductivity returned by this boundary condition,
- $T_p$ is the temperature at the centroid of the near-wall cell,
- $T_w$ is the wall temperature,
- $y_p$ is the wall-normal distance from the wall face to the centroid of the near-wall cell.

The wall function computes this effective conductivity as

\begin{equation}
    k_{eff,w} = \frac{\rho c_p u_{\tau} y_p}{T^+}\,,
\end{equation}

where:

- $\rho$ is the density
- $c_p$ is the specific heat capacity
- $u_{\tau}$ is the friction velocity
- $T^+$ is the dimensionless temperature

The molecular Prandtl number is defined as

\begin{equation}
    \mathrm{Pr} = \frac{c_p \mu}{k}\,,
\end{equation}

where:

- $\mu$ is the molecular dynamic viscosity
- $k$ is the molecular thermal conductivity

The turbulent Prandtl number $\mathrm{Pr}_t$ is provided through the material property
[!param](/LinearFVBCs/LinearFVTemperatureWallFunctionBC/Pr_t).
As in the turbulent viscosity wall function, four different formulations are supported to compute
$u_{\tau}$ and $y^+$, as defined by the [!param](/LinearFVBCs/LinearFVTemperatureWallFunctionBC/wall_treatment).

The same recommendations about mesh design and wall-normal spacing given in
[LinearFVTurbulentViscosityWallFunctionBC.md] apply here as well.

## Thermal wall-function formulation

Once $u_{\tau}$ and $y^+$ are computed, the thermal wall function evaluates the
dimensionless temperature $T^+$ using the thermal sublayer thickness
$y^+_{t}$, obtained from a fixed point solve through NS::findYplusThermal, which is dependent
on $\mathrm{Pr}$ and $\mathrm{Pr}_t$.

The thermal sublayer thickness $y^+_{t}$ is then used as the parameter
to discern whether to employ laminar sublayer thermal wall functions or turbulent
logarithmic layer thermal wall functions.

### Thermal sublayer

If

\begin{equation}
    y^+ \le y^+_{t}\,,
\end{equation}

the dimensionless temperature is defined as

\begin{equation}
    T^+ = y^+ \mathrm{Pr}\,.
\end{equation}

### Thermal logarithmic layer

If

\begin{equation}
    y^+ > y^+_{t}\,,
\end{equation}

the dimensionless temperature is defined from a Jayatilleke-type correlation as

\begin{equation}
    T^+ = \mathrm{Pr}_t
    \left[
    \frac{1}{\kappa}\ln(E y^+) + P
    \right]\,,
\end{equation}

with

\begin{equation}
P = 9.24 \left[ \left( \frac{\mathrm{Pr}}{\mathrm{Pr}_t} \right)^{0.75} - 1 \right]
\left( 1 + 0.28 \exp\left[-0.007 \frac{\mathrm{Pr}}{\mathrm{Pr}_t}\right] \right),
\end{equation}

where:

$\kappa = 0.4187$ is the von Karman constant,
$E = 9.793$ is the wall-function log law offset constant,
$P$ is the Jayatilleke temperature offset

The returned boundary value is then

\begin{equation}
    k_{eff,w} = \frac{\rho c_p u_{\tau} y_p}{T^+}.
\end{equation}

!syntax parameters /LinearFVBCs/LinearFVTemperatureWallFunctionBC

!syntax inputs /LinearFVBCs/LinearFVTemperatureWallFunctionBC

!syntax children /LinearFVBCs/LinearFVTemperatureWallFunctionBC

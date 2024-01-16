# INSFVTurbulentTemperatureWallFunction

The function sets up the equivalent heat flux of the near-wall
boundary layer when the wall temperature is set by the `T_w` parameter.

The boundary conditions are different depending on whether the centroid
of the cell near the identified boundary lies in the wall function profile.
Taking the non-dimensional wall distance as $y^+$, the three regions of the
boundary layer are identified as follows:

- Sub-laminar region: $y^+ \le 5$
- Buffer region: $y^+ \in (5, 30)$
- Logarithmic region: $y^+ \ge 30$

For the procedure of determining the non-dimensional wall distance as $y^+$
and the friction velocity $u_{\tau}$ please see
[INSFVTurbulentViscosityWallFunction](INSFVTurbulentViscosityWallFunction.md).

For the sub-laminar and logarithmic layer, the thermal diffusivity is defined
as follows:

\begin{equation}
    \alpha =
    \begin{cases}
        \alpha_l = \frac{k}{\rho c_p} & \text{if } y^+ \le 5 \\
        \alpha_t = \frac{u_{\tau} y_p}{Pr_t w_s} & \text{if } y^+ \ge 30
    \end{cases}
\end{equation}

where:

- $k$ is the thermal conductivity
- $\rho$ is the density
- $c_p$ is the specific heat at constant pressure
- $u_{\tau}$ is the friction velocity defined by law of the wall
- $y_p$ is the distance from the boundary to the centroid of the near-wall cell
- $Pr_t$ is the turbulent Prandtl number, which typically ranges between 0.3 and 0.9
- $w_s$ is a near-wall scaling factor that is defined as follows:

\begin{equation}
  w_s = \frac{1}{\kappa} \operatorname{ln}(E y^+) + J_k \,,
\end{equation}

where:

- $\kappa = 0.4187$ is the von Kármán constant
- $E = 9.793$ is a closure parameter
- $J_k$ is the Jayatilleke wall functions defined as follows:

\begin{equation}
  J_k = 9.24 \left[ \left( \frac{Pr}{Pr_t} \right)^{0.75} - 1 \right] \left( 1 + 0.28 e^{-0.007 \frac{Pr}{Pr_t}} \right) \,,
\end{equation}

where:

- $Pr$ is the Prandtl number

For the buffer layer, i.e., in $y^+ \in (5, 30)$, the thermal diffusivity
is defined via a linear blending function as follows:

\begin{equation}
  \alpha_b = \alpha_t \frac{(y^+ - 5)}{25} + \alpha_l \left[ 1 - \frac{(y^+ - 5)}{25} \right]
\end{equation}

Finally, using the thermal diffusivity, the heat flux at the wall is defined
as follows:

\begin{equation}
  q''_w = - \rho c_p \alpha \frac{T_w - T_p}{y_p} \,,
\end{equation}

where:

- $T_p$ is the temperature at the centroid of the near-wall cell
- $y_p$ is the distance from the wall to the centroid of the near-wall cell

!alert note
The thermal wall functions are only valid for regions in which the equilibrium
momentum wall functions are valid, i.e., no flow detachment, recirculation, etc.
For resolving non-equilibrium phenomena, we recommend refining the mesh.

!syntax parameters /FVBCs/INSFVTurbulentTemperatureWallFunction

!syntax inputs /FVBCs/INSFVTurbulentTemperatureWallFunction

!syntax children /FVBCs/INSFVTurbulentTemperatureWallFunction

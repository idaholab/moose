# INSFVTurbulentViscosityWallFunction

Implements wall function boundary condition for the turbulent
dynamic viscosity $\mu_t$.

The boundary conditions are different depending on where the centroid
of the cell near the identified boundary lies in the wall function profile.
Taking the non-dimensional wall distance as $y^+$, the three regions of the
boundary layer are identified as follows:

- Sub-laminar region: $y^+ \le 5$
- Buffer region: $y^+ \in (5, 30)$
- Logarithmic region: $y^+ \ge 30$

The wall function goal is to set the total viscosity at the wall $\mu_w$, decomposed as
$\mu_w = \mu + \mu_t$, such that the wall shear stress $\tau_w$ is accurately captured 
without the need of fully resolving the gradients at the near wall region. 

\begin{equation}
    \tau_w = \frac{ \mu_w u_p}{y_p} \,,
\end{equation}

where:

- $\mu_w = \mu + \mu_t$  is the total viscosity evaluated at the wall face
- $\mu_t$ is the turbulent viscosity, evaluated at the wall for the purpose of this boundary condition
- $\mu$ is the dynamic viscosity, evaluated at the wall for the purpose of this boundary condition
- $\tau_w$ is the wall-shear stress
- $u_p$ is the wall-parallel velocity at the centroid
- $y_p$ is the wall normal distance to the centroid

To impose a correct boundary condition for $\mu_t$, as seen in the Equation above, we need to compute $\tau_w$ using analytical 
relationships between the wall shear stress and the dimensionless wall distance $y^+$. For this purpose, four different
formulations are supported as defined by the [!param](/FVBCs/INSFVTurbulentViscosityWallFunction/wall_treatment) parameter.

To set the grid spacing for the first cell near the wall in your mesh, we recommend using the [RANSYPlusAux.md] auxiliary kernel. 
to estimate the dimensionless wall distance $y^+$.

## Equilibrium wall functions using a Newton solve

This treatment can be enabled by setting the parameter
[!param](/FVBCs/INSFVTurbulentViscosityWallFunction/wall_treatment) to `eq_newton`.
The treatment uses equilibrium wall functions where the following formulation is used
for the turbulent viscosity.

\begin{equation}
    \mu_t =
    \begin{cases}
        0 & \text{if } y^+ \le 5 \\
        \frac{\rho u_{\tau}^2 y_p}{u_p} - \mu & \text{if } y^+ \ge 30 \,,
    \end{cases}
\end{equation}

where:

- $\rho$ is the density
- $\mu$ is the dynamic viscosity
- $u_{\tau} = \sqrt{\frac{\tau_w}{\rho}}$ is the friction velocity and $\tau_w$ is the wall friction
- $y_p$ is the distance from the boundary to the center of the near-wall cell
- $u_p$ is the parallel velocity to the boundary computed at the center of the near-wall cell

For the buffer layer, a linear blending method is used that defines the turbulent viscosity as follows:

\begin{equation}
    \mu_t = \mu_t(y^+=30) \frac{(y^+ - 5)}{25}
\end{equation}

Note that for $y^+ = 5$ and $y^+ = 30$ we recover the sub-laminar and logarithmic profiles, respectively.

Here the standard or equilibrium law of the wall defines $y^+$ and $u_{\tau}$ as follows:

\begin{equation}
  \frac{u_p}{u_{\tau}} = \frac{1}{\kappa} \operatorname{ln}(E y^+) \,,
\end{equation}

\begin{equation}
  y^+ = \frac{\rho u_{\tau} y_p}{\mu} \,,
\end{equation}

where:

- $\mu$ is the molecular dynamic viscosity
- $E = 9.793$ is a closure parameter
- $\kappa = 0.4187$ is the von Kármán constant

In this method, we iterate on the wall function and $y^+$ to find
$u_{\tau}$ via a Newton solve. Once $u_{\tau}$ is defined, $y^+$ is
computed followed by the determination of the boundary turbulent viscosity.

!alert note
`eq_newton` solve will converge the fastest for simple flow geometries but it
may diverge for more complicated flows. Also, the code will run if the center
of the near wall cells are in the buffer layer. However, using a mesh that
contains nodes in the buffer layer is not recommended.


## Equilibrium wall functions using a fixed-point solve

This treatment is enabled by setting parameter
[!param](/FVBCs/INSFVTurbulentViscosityWallFunction/wall_treatment) to `eq_incremental`.
The method uses the same equilibrium wall treatment than the Newton solve.
However, the main difference is that, instead of computing $u_{\tau}$ for the
near wall cells, a fixed point iteration is performed in the wall functions
to find $y^+$.

!alert note
`eq_incremental` has a larger convergence radius than the Newton solve and
internal controls are added to avoid issues converging the wall function
at the buffer layer. However, it will take more iterations than the Newton
solve to converge. Using a mesh that contains nodes in the buffer layer is
not recommended.


## Equilibrium wall functions using linearized wall function

This treatment is enabled by setting parameter
[!param](/FVBCs/INSFVTurbulentViscosityWallFunction/wall_treatment) to `eq_linearized`.
The treatment uses a linearized version of the wall function, in which
a linear Taylor approximation is used for the natural logarithm.
This approximation results in a quadratic equation that is solved directly for $u_{\tau}$.
Then, $y^+$ is computed from $u_{\tau}$.

!alert note
`eq_linearized` will work fast as there is no nonlinear solve at
the near-wall region. However, the method may introduce significant
near-wall errors. The method is designed to be used in conjunction
with porous media treatment and not necessarily for free flow.

## Non-equilibrium wall functions

This treatment is enabled by setting parameter
[!param](/FVBCs/INSFVTurbulentViscosityWallFunction/wall_treatment) to `neq`.
In this case, the non-dimensional wall distance is computed from the
turbulent kinetic energy near the wall as follows:

\begin{equation}
  y^+ = \frac{\rho y_p C_{\mu}^{0.25} \sqrt{k_p}}{\mu} \,,
\end{equation}

where:

- $C_{\mu} = 0.09$ is a fitting parameter
- $k_p$ is the turbulent kinetic energy at the centroid of the near-wall cell

Then, the turbulent viscosity is defined as follows:

\begin{equation}
    \mu_t =
    \begin{cases}
        0 & \text{if } y^+ \le 5 \\
        \mu \left[ \frac{\kappa y^+}{\operatorname{ln}(E y^+)} - 1.0 \right] & \text{if } y^+ \ge 30
    \end{cases}
\end{equation}

For the buffer layer, a linear blending method is used that defines the turbulent viscosity as follows:

\begin{equation}
    \mu_t = \mu_t(y^+=30) \frac{(y^+ - 5)}{25}
\end{equation}

!alert note
`neq` should mainly be used for detached flow or other cases for which equilibrium wall
functions are not valid. One should try to use equilibrium wall functions when possible.

!syntax parameters /FVBCs/INSFVTurbulentViscosityWallFunction

!syntax inputs /FVBCs/INSFVTurbulentViscosityWallFunction

!syntax children /FVBCs/INSFVTurbulentViscosityWallFunction

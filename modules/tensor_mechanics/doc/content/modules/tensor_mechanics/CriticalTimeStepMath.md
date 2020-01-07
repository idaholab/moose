# Critical Time Step

Explicit time integration schemes are simple to implement, and they require minimal CPU
time and memory per time step. However, explicit schemes are only conditionally stable.
That is, the time step used for integration should be small than the so called
*critical time step* so the solution remains valid.

Critical time step is defined as the smallest time step which ensures that the speed
of propagating waves within an element is less than what it can theoretically transmit.
As per [!citet](hughes1987finite), critical time step ($\Delta t_{crit}$) is mathematically defined by:

\begin{equation}
\label{eqn1}
\Delta t_{crit} = \frac{2}{\omega^{G}_{max}}
\end{equation}

where $\omega^{G}_{max}$ is the maximum Eigen frequency of the entire mesh. Since it
is numerically expensive to solve for $\omega^{G}_{max}$, maximum Eigen frequency
of an element with the smallest size is usually computed as a conservative estimate. This computation,
as can be expected, depends on the element type and is further discussed for different
element types below.

# 3D Isotropic Elastic Element

For a 3D isotropic elastic element, the critical time step is given by [!cite](Askes2015):

\begin{equation}
\label{eqn2}
\Delta t^e_{crit} = min\Bigg(\frac{\Delta x}{c_d},~\frac{\Delta x}{c_s}\Bigg)
\end{equation}

where $\Delta x$ is the minimum element size, $c_d$ is the dilatational wave speed,
and $c_s$ is the shear wave speed. Further, these wave speeds are given by:

\begin{equation}
\label{eqn3}
\begin{aligned}
c_d &= \sqrt{\frac{E~(1-\nu)}{(1+\nu)~(1-2~\nu)~\rho}}\\
c_s &= \sqrt{\frac{G}{\rho}}\\
\end{aligned}
\end{equation}

where $E$ is the material elasticity modulus, $G$ is the shear modulus, $\nu$ is
the Poisson's ratio and $\rho$ is the density. Since for most applications, $\nu$ is
positive, the $\frac{\Delta x}{c_d}$ term in equation (2) is significant.

# Elastic Beam Element

Critical time step for an elastic beam element is based on the works: [!citet](Krieg1973),
[!citet](Wright1998), and [!citet](Junior2015). When either pure axial or pure shear
deformations govern, the critical time step is given by:

\begin{equation}
\label{eqn4}
\Delta t^e_{crit} = min\Bigg(\frac{\Delta x}{c_1},~\frac{\Delta x}{c_2}\Bigg)
\end{equation}

where $\Delta x$ is the minimum element size and $c_1$ and $c_2$ are given by:

\begin{equation}
\label{eqn5}
\begin{aligned}
c_1 &= \sqrt{\frac{E}{\rho}}\\
c_2 &= \sqrt{\kappa~\frac{G}{\rho}}\\
\end{aligned}
\end{equation}

where $E$, $G$, $\kappa$, and $\rho$ are the beam's young's modulus, shear modulus,
shear factor, and density, respectively.

When flexure governs, the critical time step is given by:

\begin{equation}
\label{eqn6}
\Delta t^e_{crit} = \frac{2}{\sqrt{\frac{\kappa~G~A}{\rho~I}}}\\
\end{equation}

where $A$ and $I$ are the cross-sectional area and moment of interia, respectively.
 For implementation, equations (4) and (6) are computed, and the minimum of these
 is treated as the critical time step.

!bibtex bibliography

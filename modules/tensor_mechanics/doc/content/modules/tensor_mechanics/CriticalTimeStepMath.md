# Critical Time Step

Explicit time integration schemes are simple to implement, and they require minimal CPU
time and memory per time step. However, explicit schemes are only conditionally stable.
That is, the time step used for integration should be small than the so called
_critical time step_ so the solution remains valid.

Critical time step is defined as the smallest time step which ensures that the speed
of propagating waves within an element is less than what it can theoretically transmit.
As per [!citet](Hughes1987), critical time step (\Delta t_{crit}) is mathematically defined by:

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
positive, the $\frac{\Delta x}{c_d}$ term in equation \eqref{eqn2} is significant.

# Elastic Beam Element

Critical time step for an elastic beam element is based on the work by [!citet](Krieg1973). This paper,
however, develops the mathematics of $\Delta t^e_{crit}$ specifically for an elastic plate. Hence,
the implementations for this class are generalized and adapted for an elastic beam, with the major
difference being: expressing some quantities in the [!citet](Krieg1973) work in terms of the second
moment of area ($I$) and the cross sectional area ($A$).

First, two quantities, $r_o^2$ and $r_A^2$, are defined below.

\begin{equation}
\label{eqn4}
\begin{aligned}
&r_o^2 = \frac{\Delta x^2}{2}~\Bigg(\frac{1}{c_1^2} + \frac{1}{c_2^2} - \frac{\Delta x^2}{c_1^2}~\frac{A}{I}\Bigg)\\
&r_A^2 = min\Bigg(\frac{\Delta x^2}{c_1^2},~\frac{\Delta x^2}{c_2^2}\Bigg)\\
\end{aligned}
\end{equation}

where $c_1^2 = E/\rho$ and $c_2^2 = \kappa~G/\rho$. Kappa ($\kappa$) here is the
shear factor.

## Case 1: $r_o^2 > r_A^2$

Flexural mode governs in this case.

\begin{equation}
\label{eqn5}
\Delta t^e_{crit} \leq mib\Bigg(\frac{\Delta x}{c_1},~\frac{\Delta x}{c_2}\Bigg)
\end{equation}

## Case 1: $r_o^2 \leq r_A^2$

Shear mode governs in this case.

\begin{equation}
\label{eqn6}
\begin{aligned}
&\Delta t^e_{crit} \leq \frac{\Delta x^2}{2~c_2^2}~\Bigg(\frac{c_2^2}{c_1^2} + 1 - \frac{c_2^2}{c_1^2}~\frac{\Delta x^2~A}{I}\Bigg)\\
+ \frac{I}{A~c_2^2}~\Bigg(1 - 0.5~\Big(1+\frac{c_1^2}{c_2^2}-\frac{\Delta x^2~A}{I}\Big)\Bigg)~\Bigg(1 - 0.5~\Big(1+\frac{c_2^2}{c_1^2}-\frac{c_2^2}{c_1^2}~\frac{\Delta x^2~A}{I}\Big)\Bigg)\\
\end{aligned}
\end{equation}

!bibtex bibliography

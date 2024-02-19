# WCNSFV2PSlipVelocityFunctorMaterial

This material computes the slip velocity between a dispersed phase and
a mixture phase.
The slip velocity is modeled as follows:

\begin{equation}
  \bm{v}_{slip,d} = \frac{\tau_d}{f_{drag}} \frac{\rho_d - \rho_m}{\rho_d} \bm{a} \,,
\end{equation}

where:

- $\tau_d$ is the particle relaxation time,
- $f_{drag}$ is the linear drag coefficient function,
- $\rho_d$ is the density of the dispersed phase,
- $\rho_m$ is the density of the mixture,
- $\bm{a}$ is the acceleration vector.

The particle relaxation time is modeled as follows [!cite](bilicki1990dragmodel):

\begin{equation}
  \tau_d = \frac{\rho_d d_d^2}{18 \mu_m} \,,
\end{equation}

where:

- $d_d$ is the particle diameter,
- $\mu_m$ is the mixture dynamic viscosity.

The acceleration vector is the particle acceleration vector:

\begin{equation}
  \bm{a} = \bm{g} + \frac{\bm{f}}{\rho_m} - \bm{u}_m \cdot \nabla \bm{u}_m - \frac{\partial \bm{u}_m}{dt} \,,
\end{equation}

where:

- $\bm{g}$ is the acceleration of gravity,
- $\bm{f}$ is the volumetric force,
- $\bm{u}_m$ is the mixture velocity.

!syntax parameters /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial

!syntax inputs /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial

!syntax children /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial

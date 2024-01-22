# NSFVDispersePhaseDragFunctorMaterial

This material computes the linear drag coefficient for a dispersed
phase based on the particle Reynolds number $Re_d$.
The particle Reynolds number is defined as follows:

\begin{equation}
  Re_d = \frac{\rho_d d_d \bm{u}_m}{\mu_m}
\end{equation}

where:

- $\rho_d$ is the density of the dispersed phase particles,
- $d_d$ is the characteristic diameter of the dispersed phase particles,
- $\bm{u}_m$ is the mixture velocity,
- $\mu_m$ is the mixture viscosity.

Based on this Reynolds number, the linear drag coefficient for the
dispersed phase is computed as follows [!cite](schiller1933drag):

\begin{equation}
  f_{drag} =
  \begin{cases}
      \begin{aligned}
          &1 + 0.15 Re^{0.678} & \quad &\text{if } Re \leq 1000, \\
          &0.0183 Re & \quad &\text{if } Re > 1000.
      \end{aligned}
  \end{cases}
\end{equation}

!syntax parameters /FunctorMaterials/NSFVDispersePhaseDragFunctorMaterial

!syntax inputs /FunctorMaterials/NSFVDispersePhaseDragFunctorMaterial

!syntax children /FunctorMaterials/NSFVDispersePhaseDragFunctorMaterial

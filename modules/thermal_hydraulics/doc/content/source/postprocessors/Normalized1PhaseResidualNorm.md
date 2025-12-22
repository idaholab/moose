# Normalized1PhaseResidualNorm

This post-processor computes normalized residual norms for checking convergence in [FlowChannel1Phase.md]:

!equation
\frac{\|R_\text{mass}\|_\infty}{\rho_\text{ref} A_\text{ref} h_\text{min}}

!equation
\frac{\|R_\text{momentum}\|_\infty}{\rho_\text{ref} u_\text{ref} A_\text{ref} h_\text{min}}

!equation
\frac{\|R_\text{energy}\|_\infty}{\rho_\text{ref} E_\text{ref} A_\text{ref} h_\text{min}}

where

- $p_\text{ref}$ is a reference pressure,
- $T_\text{ref}$ is a reference temperature,
- $u_\text{ref}$ is a reference velocity,
- $\rho_\text{ref} = \rho(p_\text{ref}, T_\text{ref})$ is a reference density,
- $E_\text{ref} = e(p_\text{ref}, T_\text{ref}) + \frac{1}{2} u_\text{ref}^2$ is a reference specific total energy,
- $h_\text{min}$ is the minimum element size on the channel,
- $R_\text{mass}$ is the mass equation residual,
- $R_\text{momentum}$ is the momentum equation residual,
- $R_\text{energy}$ is the energy equation residual,
- $\|\cdot\|_\infty$ is the $\ell_\infty$ norm over the channel.

!syntax parameters /Postprocessors/Normalized1PhaseResidualNorm

!syntax inputs /Postprocessors/Normalized1PhaseResidualNorm

!syntax children /Postprocessors/Normalized1PhaseResidualNorm

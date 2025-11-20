# FlowChannel1PhaseConvergence

This [Convergence/index.md] is built by [FlowChannel1Phase.md] for use by
[ComponentsConvergence.md]. It returns `CONVERGED` if all of the following are `true`
and returns `ITERATING` otherwise:

!equation
\frac{\|p^{\ell} - p^{\ell-1}\|_\infty}{p_\text{ref}} \leq \tau_p

!equation
\frac{\|T^{\ell} - T^{\ell-1}\|_\infty}{T_\text{ref}} \leq \tau_T

!equation
\frac{\|u^{\ell} - u^{\ell-1}\|_\infty}{u_\text{ref}} \leq \tau_u

!equation
\frac{\|R_\text{mass}\|_\infty}{\rho_\text{ref} A_\text{ref} h_\text{min}} \leq \tau_\text{mass}

!equation
\frac{\|R_\text{momentum}\|_\infty}{\rho_\text{ref} u_\text{ref} A_\text{ref} h_\text{min}} \leq \tau_\text{momentum}

!equation
\frac{\|R_\text{energy}\|_\infty}{\rho_\text{ref} E_\text{ref} A_\text{ref} h_\text{min}} \leq \tau_\text{energy}

where

- $p^{\ell}$ is the pressure at iteration $\ell$,
- $T^{\ell}$ is the temperature at iteration $\ell$,
- $u^{\ell}$ is the velocity at iteration $\ell$,
- $p_\text{ref}$ is a reference pressure,
- $T_\text{ref}$ is a reference temperature,
- $u_\text{ref}$ is a reference velocity,
- $\rho_\text{ref} = \rho(p_\text{ref}, T_\text{ref})$ is a reference density,
- $E_\text{ref} = e(p_\text{ref}, T_\text{ref}) + \frac{1}{2} u_\text{ref}^2$ is a reference specific total energy,
- $h_\text{min}$ is the minimum element size on the channel,
- $\tau_p$ is a tolerance for the pressure step,
- $\tau_T$ is a tolerance for the temperature step,
- $\tau_u$ is a tolerance for the velocity step,
- $R_\text{mass}$ is the mass equation residual,
- $R_\text{momentum}$ is the momentum equation residual,
- $R_\text{energy}$ is the energy equation residual,
- $\|\cdot\|_\infty$ is the $\ell_\infty$ norm over the channel.

!syntax parameters /Convergence/FlowChannel1PhaseConvergence

!syntax inputs /Convergence/FlowChannel1PhaseConvergence

!syntax children /Convergence/FlowChannel1PhaseConvergence

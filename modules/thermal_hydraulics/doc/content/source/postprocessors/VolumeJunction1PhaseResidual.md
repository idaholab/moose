# VolumeJunction1PhaseResidual

This post-processor computes the following normalized residual norms for checking convergence in [VolumeJunction1Phase.md]:

!equation
\frac{|R_\text{mass}|}{\rho_\text{ref} V}

!equation
\frac{|R_\text{x-momentum}|}{\rho_\text{ref} u_\text{ref} V}

!equation
\frac{|R_\text{y-momentum}|}{\rho_\text{ref} u_\text{ref} V}

!equation
\frac{|R_\text{z-momentum}|}{\rho_\text{ref} u_\text{ref} V}

!equation
\frac{|R_\text{energy}|}{\rho_\text{ref} E_\text{ref} V}

where

- $p_\text{ref}$ is a reference pressure,
- $T_\text{ref}$ is a reference temperature,
- $u_\text{ref}$ is a reference velocity,
- $\rho_\text{ref} = \rho(p_\text{ref}, T_\text{ref})$ is a reference density,
- $E_\text{ref} = e(p_\text{ref}, T_\text{ref}) + \frac{1}{2} u_\text{ref}^2$ is a reference specific total energy,
- $V$ is the junction volume,
- $R_\text{mass}$ is the mass equation residual,
- $R_\text{x-momentum}$ is the x-momentum equation residual,
- $R_\text{y-momentum}$ is the y-momentum equation residual,
- $R_\text{z-momentum}$ is the z-momentum equation residual, and
- $R_\text{energy}$ is the energy equation residual.

!syntax parameters /Postprocessors/VolumeJunction1PhaseResidual

!syntax inputs /Postprocessors/VolumeJunction1PhaseResidual

!syntax children /Postprocessors/VolumeJunction1PhaseResidual

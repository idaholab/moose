# VolumeJunction1Phase

This is a [flow junction](component_groups/flow_junction.md) that has a volume
and can connect 2 or more [FlowChannel1Phase.md] components in any orientation.

## Formulation

See [modules/thermal_hydraulics/theory_manual/vace_model/volume_junction.md] for
the theoretical formulation.

### Form Losses

!template load file=volume_junction_1phase_formulation_formloss.md.template name=VolumeJunction1Phase

## Usage id=usage

!template load file=flow_junction_usage.md.template name=VolumeJunction1Phase

A form loss coefficient $K$ may be specified using the parameter
[!param](/Components/VolumeJunction1Phase/K).
The parameter [!param](/Components/VolumeJunction1Phase/A_ref) is the reference
cross-sectional area $A_\text{ref}$ used in [formloss_momentum] and [formloss_energy]. If it is
not provided, the cross-sectional area of the first connection in
[!param](/Components/VolumeJunction1Phase/connections) is used.

!alert note title=Connection order matters when using form loss
The order of connections in [!param](/Components/VolumeJunction1Phase/connections)
has an impact when using form loss, since some quantities in [formloss_momentum] and [formloss_energy]
are taken from the *first* connection.

!template load file=volume_junction_1phase_usage.md.template name=VolumeJunction1Phase

!alert tip title=Visualizing junction solutions
Since junction variables are located on the mesh, they can visualized alongside the other field variables, such as from pipes. In Paraview, the junction can be made more visible using the "point size" parameter.

## Convergence

If using [ComponentsConvergence.md], a Convergence object of type [MultiPostprocessorConvergence.md] is used that returns `CONVERGED` if all of the following are `true` and returns `ITERATING` otherwise:

!equation
\frac{|p^{\ell} - p^{\ell-1}|}{p_\text{ref}} \leq \tau_p

!equation
\frac{|T^{\ell} - T^{\ell-1}|}{T_\text{ref}} \leq \tau_T

!equation
\frac{|u_x^{\ell} - u_x^{\ell-1}|}{u_\text{ref}} \leq \tau_u

!equation
\frac{|u_y^{\ell} - u_y^{\ell-1}|}{u_\text{ref}} \leq \tau_u

!equation
\frac{|u_z^{\ell} - u_z^{\ell-1}|}{u_\text{ref}} \leq \tau_u

!equation
\frac{|R_\text{mass}|}{\rho_\text{ref} V} \leq \tau_\text{mass}

!equation
\frac{|R_\text{x-momentum}|}{\rho_\text{ref} u_\text{ref} V} \leq \tau_\text{momentum}

!equation
\frac{|R_\text{y-momentum}|}{\rho_\text{ref} u_\text{ref} V} \leq \tau_\text{momentum}

!equation
\frac{|R_\text{z-momentum}|}{\rho_\text{ref} u_\text{ref} V} \leq \tau_\text{momentum}

!equation
\frac{|R_\text{energy}|}{\rho_\text{ref} E_\text{ref} V} \leq \tau_\text{energy}

where

- $p^{\ell}$ is the pressure at iteration $\ell$,
- $T^{\ell}$ is the temperature at iteration $\ell$,
- $u_x^{\ell}$ is the x-velocity at iteration $\ell$,
- $u_y^{\ell}$ is the y-velocity at iteration $\ell$,
- $u_z^{\ell}$ is the z-velocity at iteration $\ell$,
- $p_\text{ref}$ is a reference pressure,
- $T_\text{ref}$ is a reference temperature,
- $u_\text{ref}$ is a reference velocity,
- $\rho_\text{ref} = \rho(p_\text{ref}, T_\text{ref})$ is a reference density,
- $E_\text{ref} = e(p_\text{ref}, T_\text{ref}) + \frac{1}{2} u_\text{ref}^2$ is a reference specific total energy,
- $V$ is the junction volume,
- $\tau_p$ is a tolerance for the pressure step,
- $\tau_T$ is a tolerance for the temperature step,
- $\tau_u$ is a tolerance for the velocity step,
- $R_\text{mass}$ is the mass equation residual,
- $R_\text{x-momentum}$ is the x-momentum equation residual,
- $R_\text{y-momentum}$ is the y-momentum equation residual,
- $R_\text{z-momentum}$ is the z-momentum equation residual,
- $R_\text{energy}$ is the energy equation residual,
- $\tau_\text{mass}$ is the mass equation tolerance,
- $\tau_\text{momentum}$ is the momentum equation tolerance, and
- $\tau_\text{energy}$ is the energy equation tolerance.

!alert note title=One iteration required
This object always returns `ITERATING` for the first iteration due to the usage of step criteria.

The following parameters are relevant for these checks:

- [!param](/Components/VolumeJunction1Phase/p_ref): The reference pressure $p_\text{ref}$,
- [!param](/Components/VolumeJunction1Phase/T_ref): The reference temperature $T_\text{ref}$,
- [!param](/Components/VolumeJunction1Phase/vel_ref): The reference velocity $u_\text{ref}$,
- [!param](/Components/VolumeJunction1Phase/p_rel_step_tol): The relative step tolerance for pressure $\tau_p$,
- [!param](/Components/VolumeJunction1Phase/T_rel_step_tol): The relative step tolerance for temperature $\tau_T$,
- [!param](/Components/VolumeJunction1Phase/vel_rel_step_tol): The relative step tolerance for velocity $\tau_u$,
- [!param](/Components/VolumeJunction1Phase/mass_res_tol): The mass residual tolerance $\tau_\text{mass}$,
- [!param](/Components/VolumeJunction1Phase/momentum_res_tol): The momentum residual tolerance $\tau_\text{momentum}$,
- [!param](/Components/VolumeJunction1Phase/energy_res_tol): The energy residual tolerance $\tau_\text{energy}$.

## Implementation Notes

The junction variables are technically field variables that live on a block consisting of a single `NodeElem`. Originally, junction variables were scalar variables, but it was found that this was extremely costly due to the sparsity pattern requirements, thus making simulations involving large numbers of these components to be very slow to initialize.

!syntax parameters /Components/VolumeJunction1Phase

!syntax inputs /Components/VolumeJunction1Phase

!syntax children /Components/VolumeJunction1Phase

!bibtex bibliography

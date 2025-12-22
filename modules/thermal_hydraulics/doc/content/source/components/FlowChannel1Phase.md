# FlowChannel1Phase

This component is a single-phase [flow channel](component_groups/flow_channel.md).

## Usage

!template load file=geometrical_component_usage.md.template name=FlowChannel1Phase

!alert note
[!param](/Components/FlowChannel1Phase/orientation) can only be used to specify a single
direction and thus cannot be used to specify bends in a flow channel.

Each end of a flow channel must be connected to either a
[boundary](component_groups/flow_boundary.md) or a
[junction](component_groups/flow_junction.md)
(see [#mesh_blocks] for the boundary naming conventions).

!template load file=flow_channel_usage.md.template name=FlowChannel1Phase

Initial conditions are specified for pressure, temperature, and velocity with
the following parameters:

- [!param](/Components/FlowChannel1Phase/initial_p)
- [!param](/Components/FlowChannel1Phase/initial_T)
- [!param](/Components/FlowChannel1Phase/initial_vel)

This component offers options to output quantities via vector post-processors:

- [!param](/Components/FlowChannel1Phase/vpp_vars): Creates an [ElementValueSampler.md] with a vector for each of the listed variables (see below for a list of variables).
- [!param](/Components/FlowChannel1Phase/create_flux_vpp): Creates a [NumericalFlux3EqnInternalValues.md], which creates a vector for each numerical flux component (mass, momentum, energy) at the internal sides.

!syntax parameters /Components/FlowChannel1Phase

## Mesh id=mesh

### Axial Discretization id=mesh_axial

!template load file=geometrical_component_mesh.md.template name=FlowChannel1Phase

### Blocks and Boundaries id=mesh_blocks

!template load file=flow_channel_mesh.md.template name=FlowChannel1Phase

## Variables

The following solution variables are created on the flow channel:

| Variable | Symbol | Description |
| :- | :- | :- |
| `rhoA` | $\rho A$ | Mass per unit length \[kg/m\] |
| `rhouA` | $\rho u A$ | Momentum per unit length; mass flow rate \[kg/s\] |
| `rhoEA` | $\rho E A$ | Energy per unit length \[J/m\] |

The following auxiliary variables are created on the flow channel:

| Variable | Symbol | Description |
| :- | :- | :- |
| `A` | $A$ | Cross-sectional area \[m$^2$\] (piecewise constant) |
| `A_linear` | $A$ | Cross-sectional area \[m$^2$\] (piecewise linear) |
| `P_hf` | $P_\text{heat}$ | Heated perimeter \[m\] |
| `vel_x` | $u_x$ | Velocity component along the x-axis \[m/s\] (if specified to output vector-valued velocity) |
| `vel_y` | $u_y$ | Velocity component along the y-axis \[m/s\] (if specified to output vector-valued velocity) |
| `vel_z` | $u_z$ | Velocity component along the z-axis \[m/s\] (if specified to output vector-valued velocity) |
| `vel` | $u$ | Velocity component along flow channel direction \[m/s\] (if specified not to output vector-valued velocity) |
| `rho` | $\rho$ | Density \[kg/m$^3$\] |
| `p` | $p$ | Pressure \[Pa\] |
| `T` | $T$ | Temperature \[K\] |
| `v` | $v$ | Specific volume \[m$^3$/kg\] |
| `e` | $e$ | Specific internal energy \[J/kg\] |
| `H` | $H$ | Specific total enthalpy \[J/kg\] |

## Material Properties

The following material properties are created on the flow channel:

| Material Property | Symbol | Description |
| :- | :- | :- |
| `direction` | $\mathbf{d}$ | Flow channel direction vector \[-\] |
| `rhoA` | $\rho A$ | Mass per unit length \[kg/m\] (slope-reconstructed) |
| `rhouA` | $\rho u A$ | Momentum per unit length; mass flow rate \[kg/s\] (slope-reconstructed) |
| `rhoEA` | $\rho E A$ | Energy per unit length \[J/m\] (slope-reconstructed) |
| `vel` | $u$ | Velocity component along flow channel direction \[m/s\] |
| `rho` | $\rho$ | Density \[kg/m$^3$\] |
| `p` | $p$ | Pressure \[Pa\] |
| `T` | $T$ | Temperature \[K\] |
| `v` | $v$ | Specific volume \[m$^3$/kg\] |
| `e` | $e$ | Specific internal energy \[J/kg\] |
| `h` | $h$ | Specific enthalpy \[J/kg\] |
| `H` | $H$ | Specific total enthalpy \[J/kg\] |
| `c` | $c$ | Sound speed \[m/s\] |
| `cp` | $c_p$ | Isobaric specific heat capacity \[J/(kg-K)\] |
| `cv` | $c_v$ | Isochoric specific heat capacity \[J/(kg-K)\] |
| `k` | $k$ | Thermal conductivity \[W/(m-K)\] |
| `mu` | $\mu$ | Dynamic viscosity \[Pa-s\] |
| `f_D` | $f_D$ | Darcy friction factor \[-\] |
| `D_h` | $D_h$ | Hydraulic diameter \[m\] |
| `q_wall` | $q_\text{wall}$ | Wall heat flux \[W/m$^2$\] (if no connected heat transfer) |

## Formulation

See [!cite](relap7theory) for a description of the single-phase flow formulation.

## Convergence

If using [ComponentsConvergence.md], a Convergence object of type [MultiPostprocessorConvergence.md] is used that returns `CONVERGED` if all of the following are `true` and returns `ITERATING` otherwise:

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
- $A_\text{ref} = A(\mathbf{x}_\text{mid})$ is a reference cross-sectional area, evaluated at the midpoint $\mathbf{x}_\text{mid}$ of the channel,
- $h_\text{min}$ is the minimum element size on the channel,
- $\tau_p$ is a tolerance for the pressure step,
- $\tau_T$ is a tolerance for the temperature step,
- $\tau_u$ is a tolerance for the velocity step,
- $R_\text{mass}$ is the mass equation residual,
- $R_\text{momentum}$ is the momentum equation residual,
- $R_\text{energy}$ is the energy equation residual,
- $\tau_\text{mass}$ is the mass equation tolerance,
- $\tau_\text{momentum}$ is the momentum equation tolerance,
- $\tau_\text{energy}$ is the energy equation tolerance, and
- $\|\cdot\|_\infty$ is the $\ell_\infty$ norm over the channel.

!alert note title=One iteration required
This object always returns `ITERATING` for the first iteration due to the usage of step criteria.

The following parameters are relevant for these checks:

- [!param](/Components/FlowChannel1Phase/p_ref): The reference pressure $p_\text{ref}$,
- [!param](/Components/FlowChannel1Phase/T_ref): The reference temperature $T_\text{ref}$,
- [!param](/Components/FlowChannel1Phase/vel_ref): The reference velocity $u_\text{ref}$,
- [!param](/Components/FlowChannel1Phase/p_rel_step_tol): The relative step tolerance for pressure $\tau_p$,
- [!param](/Components/FlowChannel1Phase/T_rel_step_tol): The relative step tolerance for temperature $\tau_T$,
- [!param](/Components/FlowChannel1Phase/vel_rel_step_tol): The relative step tolerance for velocity $\tau_u$,
- [!param](/Components/FlowChannel1Phase/mass_res_tol): The mass residual tolerance $\tau_\text{mass}$,
- [!param](/Components/FlowChannel1Phase/momentum_res_tol): The momentum residual tolerance $\tau_\text{momentum}$,
- [!param](/Components/FlowChannel1Phase/energy_res_tol): The energy residual tolerance $\tau_\text{energy}$.

!syntax inputs /Components/FlowChannel1Phase

!syntax children /Components/FlowChannel1Phase

!bibtex bibliography

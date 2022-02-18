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

!syntax inputs /Components/FlowChannel1Phase

!syntax children /Components/FlowChannel1Phase

!bibtex bibliography

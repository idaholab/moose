# VolumeJunctionCoupledFlux1Phase

This component couples a [VolumeJunction1Phase.md] to another application via a [MultiApp](MultiApps/index.md) by applying a numerical flux computed using the junction state and the external state.

The junction state $\mathbf{U}_\text{J}$ is

!equation
\mathbf{U}_\text{J} = \left[\begin{array}{c}
  \rho_\text{J} A_\text{coupled}\\
  \rho_\text{J} u_\text{J} A_\text{coupled}\\
  \rho_\text{J} v_\text{J} A_\text{coupled}\\
  \rho_\text{J} w_\text{J} A_\text{coupled}\\
  \rho_\text{J} E_\text{J} A_\text{coupled}\\
  C_{0,\text{J}} A_\text{coupled}\\
  \vdots\\
  C_{N_C-1,\text{J}} A_\text{coupled}
  \end{array}\right]

where

- $A_\text{coupled}$ is the coupling area,
- $\rho_\text{J}$ is the junction density,
- $[u_\text{J}, v_\text{J}, w_\text{J}]$ is the junction velocity,
- $E_\text{J}$ is the junction specific total energy, and
- $C_{0,\text{J}},\ldots,C_{N_C-1,\text{J}}$ are the concentrations of $N_C$ passive species (if any).

The external state $\mathbf{U}_\text{ext}$ is defined by a pressure $p_\text{ext}$, a temperature $T_\text{ext}$, and the passive concentrations $C_{0,\text{ext}},\ldots,C_{N_C-1,\text{ext}}$, while assuming the velocity is zero:

!equation
\mathbf{U}_\text{ext} = \left[\begin{array}{c}
  \rho_\text{ext} A_\text{coupled}\\
  0\\
  0\\
  0\\
  \rho_\text{ext} e_\text{ext} A_\text{coupled}\\
  C_{0,\text{ext}} A_\text{coupled}\\
  \vdots\\
  C_{N_C-1,\text{ext}} A_\text{coupled}
  \end{array}\right]

where $\rho_\text{ext} = \rho(p_\text{ext}, T_\text{ext})$ and $e_\text{ext} = e(p_\text{ext}, T_\text{ext})$.

The numerical flux vector is computed as

!equation
\mathbf{F} = \mathcal{F}(\mathbf{U}_\text{J}, \mathbf{U}_\text{ext}, \mathbf{n}_\text{ext,J})

where $\mathcal{F}$ is the 3-D numerical flux function and $\mathbf{n}_\text{ext,J}$ is the normal vector from the exterior application to the junction.

The following table gives the post-processors created by this component, where `<suffix>` is the suffix provided by [!param](/Components/VolumeJunctionCoupledFlux1Phase/pp_suffix) and `<Ci_name>` is the name of $C_i$:

| Name | Symbol | Type | Transfer Direction |
| :- | :- | :- | :- |
| `mass_rate_<suffix>` | $F_\text{mass}$ | [VolumeJunctionCoupledFlux1PhasePostprocessor.md] | To other app |
| `energy_rate_<suffix>` | $F_\text{energy}$ | [VolumeJunctionCoupledFlux1PhasePostprocessor.md] | To other app |
| `<Ci_name>_rate_<suffix>` | $F_{C_i}$ | [VolumeJunctionCoupledFlux1PhasePostprocessor.md] | To other app |
| `p_<suffix>` | $p_\text{ext}$ | [Receiver.md] | From other app |
| `T_<suffix>` | $T_\text{ext}$ | [Receiver.md] | From other app |
| `<Ci_name>_<suffix>` | $C_{i,\text{ext}}$ | [Receiver.md] | From other app |

If this application is the main application, then the [!param](/Components/VolumeJunctionCoupledFlux1Phase/multi_app) parameter should be provided, which
creates [MultiAppPostprocessorTransfer.md] objects; otherwise the other application is responsible for creating the transfers.

!syntax parameters /Components/VolumeJunctionCoupledFlux1Phase

!syntax inputs /Components/VolumeJunctionCoupledFlux1Phase

!syntax children /Components/VolumeJunctionCoupledFlux1Phase

!bibtex bibliography

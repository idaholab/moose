# VolumeJunctionCoupledFlux1Phase

This component couples a [VolumeJunction1Phase.md] to another application via a [MultiApp](MultiApps/index.md) by applying a numerical flux computed using the junction state and the external state.

The junction state $\mathbf{U}_\text{J}$ is

!equation
\mathbf{U} = \left[\begin{array}{c}
  \rho_\text{J} A_\text{coupled}\\
  \rho_\text{J} u_\text{J} A_\text{coupled}\\
  \rho_\text{J} v_\text{J} A_\text{coupled}\\
  \rho_\text{J} w_\text{J} A_\text{coupled}\\
  \rho_\text{J} E_\text{J} A_\text{coupled}
  \end{array}\right]

where $A_\text{coupled}$ is the coupling area; see [VolumeJunction1Phase.md] for the definition of junction quantities.

The external state $\mathbf{U}_\text{ext}$ is defined by a pressure $p_\text{ext}$ and a temperature $T_\text{ext}$, while assuming the velocity is zero:

!equation
\mathbf{U}_\text{ext} = \left[\begin{array}{c}
  \rho_\text{ext} A_\text{coupled}\\
  0\\
  0\\
  0\\
  \rho_\text{ext} e_\text{ext} A_\text{coupled}
  \end{array}\right]

where $\rho_\text{ext} = \rho(p_\text{ext}, T_\text{ext})$ and $e_\text{ext} = e(p_\text{ext}, T_\text{ext})$.

The numerical flux vector is computed as

!equation
\mathbf{F} = \mathcal{F}(\mathbf{U}_\text{J}, \mathbf{U}_\text{ext}, \mathbf{n}_\text{ext,J})

where $\mathcal{F}$ is the 3-D numerical flux function and $\mathbf{n}_\text{ext,J}$ is the normal vector from the exterior application to the junction.

The following table gives the post-processors created by this component, where `<suffix>` is the suffix provided by [!param](/Components/VolumeJunctionCoupledFlux1Phase/pp_suffix):

| Name | Description | Type | Transfer Direction |
| :- | :- | :- | :- |
| `mass_rate_<suffix>` | $F_\text{mass}$ | [VolumeJunctionCoupledFlux1PhasePostprocessor.md] | To other app |
| `energy_rate_<suffix>` | $F_\text{energy}$ | [VolumeJunctionCoupledFlux1PhasePostprocessor.md] | To other app |
| `p_<suffix>` | $p_\text{ext}$ | [Receiver.md] | From other app |
| `T_<suffix>` | $T_\text{ext}$ | [Receiver.md] | From other app |

If this application is the main application, then the [!param](/Components/VolumeJunctionCoupledFlux1Phase/multi_app) parameter should be provided, which
creates [MultiAppPostprocessorTransfer.md] objects; otherwise the other application is resonsible for creating the transfers.

!syntax parameters /Components/VolumeJunctionCoupledFlux1Phase

!syntax inputs /Components/VolumeJunctionCoupledFlux1Phase

!syntax children /Components/VolumeJunctionCoupledFlux1Phase

!bibtex bibliography

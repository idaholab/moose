# CoupledHeatTransferAction

## Description

This action creates many of the low-level objects needed to perform a convective
heat transfer multiphysics coupling with a [1-D flow channel](component_groups/flow_channel.md).
It assumes that the domain external to the flow channel is the master-app,
and the application(s) containing the flow channel(s) are the sub-apps.
Hereafter the external domain is referred to as the "solid side", and the flow
channel is referred to as the "fluid side".

This action is suitable for coupling to either single-phase or multi-phase
flow channels. The general formulation it uses for the heat fluxes *to* the solid
side is

!equation id=solid_side_heat_flux
q = \sum\limits_k \kappa_k \mathcal{H}_k (T_k - T) \eqc

where $k$ is the fluid phase index, $T$ is the solid-side temperature, $T_k$ is the
temperature of phase $k$, $\mathcal{H}_k$ is the heat transfer coefficient of
phase $k$, and $\kappa_k$ is the wall contact fraction of phase $k$.

The action creates the following MOOSE objects:

- A [/CoupledConvectiveHeatFluxBC.md] on the boundaries provided in the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/boundary) parameter,
  which implements the heat flux boundary condition described by [solid_side_heat_flux].
- A [/NearestPointLayeredSideAverage.md] user object to compute the average of
  the solid-side temperature variable (specified by the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/T) parameter) for
  each axial layer, corresponding to each of the elements in the flow channel.
  The layers are determined by the parameters
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/position),
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/orientation),
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/rotation),
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/length), and
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/n_elems),
  which correspond to the parameters of the same names in the coupled flow
  channel. These parameters should be copied from the sub input file.
- A [/MultiAppUserObjectTransfer.md] to transfer the solid-side temperature
  layer averages into the sub-app variable provided via the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/T_wall) parameter.
- A [/MultiAppUserObjectTransfer.md] to transfer each of the phase temperature
  layer averages, provided by the user object(s) specified in the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/T_fluid_user_objects)
  parameter, into the master-app variables specified by the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/T_fluid) parameter.
- A [/MultiAppUserObjectTransfer.md] to transfer each of the phase heat transfer coefficient
  layer averages, provided by the user object(s) specified in the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/htc_user_objects)
  parameter, into the master-app variables specified by the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/htc) parameter.
- If multi-phase, a [/MultiAppUserObjectTransfer.md] to transfer each of the phase wall contact fraction
  layer averages, provided by the user object(s) specified in the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/kappa_user_objects)
  parameter, into the master-app variables specified by the
  [!param](/CoupledHeatTransfers/CoupledHeatTransferAction/kappa) parameter.

## Instructions for the Master Input File

The following must be defined in the master-app input file:

- `AuxVariables` for the fluid temperature(s), heat transfer coefficient(s), and
  wall contact fractions (if multi-phase). For example,

  !listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/master.i
           block=AuxVariables
           link=False

  It is recommended that the FE type match that of the corresponding variables
  in the coupled flow channel.

- `MultiApps` for the flow-channel sub-app(s). For example,

  !listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/master.i
           block=MultiApps
           link=False

## Instructions for the Sub Input File

The following must be defined in the sub-app input file:

- `AuxVariables` for the fluid temperature(s), heat transfer coefficient(s), and
  wall contact fractions (if multi-phase). For example, for THM's single-phase
  flow model, the fluid temperature is already available as an aux variable, but
  the wall heat transfer coefficient is an AD material property. Thus a [/MaterialRealAux.md]
  must be created:

  !listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i
           block=AuxVariables
           link=False

  !listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i
           block=AuxKernels
           link=False

- [/LayeredAverage.md] user objects for the fluid temperature(s), heat transfer coefficient(s),
  and wall contact fractions (if multi-phase). For example,

  !listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i
           block=UserObjects
           link=False

- A component to implement the flow channel heat source corresponding to [solid_side_heat_flux].
  For example, for THM's single-phase flow model, the [/HeatTransferFromExternalAppTemperature1Phase.md]
  component is used.

!syntax parameters /CoupledHeatTransfers/CoupledHeatTransferAction

!syntax inputs /CoupledHeatTransfers/CoupledHeatTransferAction

## Example

Heat conduction input file:

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/master.i

Flow channel input file:

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i

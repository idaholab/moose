# Coupled Heat Transfer Action

## Description

The `CoupledHeatTransferAction` action sets up all of the necessary objects for
doing transfer between heat conduction and thermal hydraulics module. It should
be used with the heat conduction problem in the master-app. It is setting up the following objects:

- A convective boundary condition on the boundaries provided in the `boundary` parameter
  using the heat transfer coefficient variable `htc` and the fluid temperature `T_fluid`.
- A UserObject of type `LayeredSideAverage` operating on the solid temperature defined
  in the `T` parameter, using a number of `num_layers` layers in the directions provided
  in `direction`.
- A `MultiAppUserObjectTransfer` to the sub-app defined in the `multi_app` parameter.
  This will transfer the resulting solid surface temperature into the sub-app variable
  provided in the `T_wall` parameter.
- A `MultiAppUserObjectTransfer` from the sub-app defined in the `multi_app` parameter.
  This will transfer the fluid temperature calculated in the sub-app by the user object defined by
  `T_fluid_user_object` into the AuxVariable defined by `T_fluid`.
- A `MultiAppUserObjectTransfer` from the sub-app defined in the `multi_app` parameter.
  This will transfer the heat transfer coefficient calculated in the sub-app by the user object
  defined by `htc_user_object` into the AuxVariable defined by `htc`.

To use this action, the following objects must be defined in the master-app input file:

- `AuxVariables` for the fluid temperature and heat transfer coefficient.

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/master.i
         block=AuxVariables
         link=False


- `MultiApps` for the THM sub-app

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/master.i
         block=MultiApps
         link=False

The following must be defined in the THM sub-app:

- `AuxVariables` for the heat transfer coefficient associated with a `ADMaterialRealAux` AuxKernel.
  In THM, the heat transfer coefficient is defined as a material, but an AuxVariable is needed
  for the transfers.

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i
         block=AuxVariables
         link=False


 !listing tthermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i
          block=AuxKernels
          link=False

- `LayeredAverage` user-objects for the fluid temperature and the heat transfer coefficient.

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i
         block=UserObjects
         link=False

- `HeatTransferFromExternalAppTemperature1Phase` component. If `T_ext` is not set, then the name of
   solid surface temperature is `T_wall`.


!syntax parameters /CoupledHeatTransfers/CoupledHeatTransferAction

!syntax inputs /CoupledHeatTransfers/CoupledHeatTransferAction


## Example

Heat conduction input file:

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/master.i

THM input file:

!listing thermal_hydraulics/test/tests/actions/coupled_heat_transfer_action/sub.i

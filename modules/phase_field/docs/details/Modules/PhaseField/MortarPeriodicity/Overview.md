The `MortarPeriodicity` Action assists users in setting up mortar based periodicity
constraints. Its primary application is setting up an [EqualGradientConstraint] which,
when used on the displacement variables, implemnts a strain periodicity which is
usefule for representative volume element microstructure simulations.

A [MortarPeriodicMesh](../../../Mesh/MortarPeriodicMesh.moose) must be used with this action.

![Example](https://cloud.githubusercontent.com/assets/202302/14893652/785e9918-0d2d-11e6-9623-b21a66fc58d5.gif)

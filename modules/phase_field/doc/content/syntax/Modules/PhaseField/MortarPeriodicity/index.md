# MortarPeriodicity System

The `MortarPeriodicity` Action assists users in setting up mortar based periodicity
constraints. Its primary application is setting up an [EqualGradientConstraint](/EqualGradientConstraint.md) which,
when used on the displacement variables, implemnts a strain periodicity which is
useful for representative volume element microstructure simulations.

!media phase_field/mortar_periodic_strain.gif

!syntax list /Modules/PhaseField/MortarPeriodicity objects=True actions=False subsystems=False

!syntax list /Modules/PhaseField/MortarPeriodicity objects=False actions=False subsystems=True

!syntax list /Modules/PhaseField/MortarPeriodicity objects=False actions=True subsystems=False

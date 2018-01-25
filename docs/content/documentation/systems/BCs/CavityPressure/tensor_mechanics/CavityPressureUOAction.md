# Cavity Pressure UserObject Action
!syntax description /BCs/CavityPressure/CavityPressureUOAction

## Description
The `CavityPressurePPAction` is one of three actions in the Cavity Pressure Action system which are intended to be used concurrently.
The intention of the Cavity Pressure Action system is to reduce the number of input file blocks required to compute the pressure exerted by a gas contained in an internal volume.

## Constructed MooseObjects
!include /CavityPressure/index.md start=The Cavity Pressure Action system consists end=!syntax objects

## Example Input File Syntax
!listing modules/combined/test/tests/cavity_pressure/cavity_pressure.i block=BCs/CavityPressure

Postprocessors for both the average temperature and the internal volume are also required for the Cavity Pressure Action system. Note that the name of the postprocessors correspond to the arguments for the parameters `temperature` and `internal_volume` in the `CavityPressure` block.
!listing modules/combined/test/tests/cavity_pressure/cavity_pressure.i block=Postprocessors/aveTempInterior

!listing modules/combined/test/tests/cavity_pressure/cavity_pressure.i block=Postprocessors/internalVolume

!syntax parameters /BCs/CavityPressure/CavityPressurePPAction

!syntax parameters /BCs/CavityPressure/CavityPressureUOAction

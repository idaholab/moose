# StepUserObject

!syntax description /UserObjects/StepUserObject

## Description

`StepUserObject` is a general user object that performs basic computations to provide a
loading step given a simulation time step value or a simulation time step value given a
loading step. This user object is used to interface with [AbaqusUMATStress](/AbaqusUMATStress.md)
and [AbaqusUExternalDB](/AbaqusUExternalDB.md) to provide step information to user routines.
`StepUserObject` can also interface with [StepPeriod](/StepPeriod.md) to enable/disable
boundary conditions and constraints according the user-defined loading steps.


## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/umat/steps/elastic_temperature_steps_uo.i block=UserObjects/step_uo

!syntax parameters /UserObjects/StepUserObject

!syntax inputs /UserObjects/StepUserObject

!syntax children /UserObjects/StepUserObject

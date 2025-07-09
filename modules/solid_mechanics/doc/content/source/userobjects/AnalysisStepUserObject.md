# AnalysisStepUserObject

!syntax description /UserObjects/AnalysisStepUserObject

## Description

`AnalysisStepUserObject` is a general user object that performs basic computations to provide an
analysis step given a simulation time step value or a simulation time step value given an
analysis step. This user object is used to interface with [AbaqusUMATStress](/AbaqusUMATStress.md)
and [AbaqusUExternalDB](/AbaqusUExternalDB.md) to provide analysis step information to user routines.
`AnalysisStepUserObject` can also interface with [StepPeriod](/StepPeriod.md) to enable/disable
boundary conditions and constraints according the user-defined analysis steps.


## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/umat/analysis_steps/elastic_temperature_steps_uo.i block=UserObjects/step_uo

!syntax parameters /UserObjects/AnalysisStepUserObject

!syntax inputs /UserObjects/AnalysisStepUserObject

!syntax children /UserObjects/AnalysisStepUserObject

# GrainGrowthAction

!syntax description /Modules/PhaseField/GrainGrowth/GrainGrowthAction

## Overview

This action creates the variables, kernels, auxvariables, and auxkernels needed to run a polycrystal simulation. All of these various objects could be added manually in an input file, but the input file would be very long and would require many changes in order to change the number of order parameters being used to represent the grains. This action vastly simplifies the input file syntax.

The required input parameter `op_num` defines how many order parameters will be generated. Each order parameter is named based on the `var_name_base` parameter; e.g. for `op_num` = 3, `var_name_base` = `gr`, three nonlinear variables are created (`gr0`, `gr1`, and `gr2`).

When using the action, you can choose to either use the automatic differentiation (AD) versions of the kernels or the versions with hand-coded Jacobians with the parameter `use_automatic_differentiation`. It defaults to 'false'.

The following objects are created:

+Variables+

- A total of `op_num` variables

+AuxVariables+

- `bnds`, used to visualize grain boundaries

+Kernels+

- One per variable, with a total of `op_num`

  - [TimeDerivative](/TimeDerivative.md) or [ADTimeDerivative](/ADTimeDerivative.md)
  - [ACGrGrPoly](/ACGrGrPoly.md) or [ADGrainGrowth](/ADGrainGrowth.md)
  - [ACInterface](/ACInterface.md) or [ADACInterface](/ADACInterface.md)

+AuxKernels+

- [BndsCalcAux](/BndsCalcAux.md), used to define grain boundary visualization AuxVariable `bnds`

## Example Input File Syntax

The `GrainGrowthAction` is created as shown below.

!listing modules/phase_field/test/tests/actions/grain_growth.i block=GlobalParams

!listing modules/phase_field/test/tests/actions/grain_growth.i block=Modules

!syntax parameters /Modules/PhaseField/GrainGrowth/GrainGrowthAction

!syntax children /Modules/PhaseField/GrainGrowth/GrainGrowthAction

# GrainGrowthLinearizedInterfaceAction

!syntax description /Modules/PhaseField/GrainGrowthLinearizedInterface/GrainGrowthLinearizedInterfaceAction

## Overview

This action creates the variables, kernels, auxvariables, auxkernels, bounds, and materials needed to implement a polycrystal simulation using a linearized interface. All of these various objects could be added manually in an input file, but the input file would be very long and would require many changes in order to change the number of order parameters being used to represent the grains. This action vastly simplifies the input file syntax and makes the syntax for a traditional phase field grain growth model and one using a linearized interface vey similar.

The required input parameter `op_num` defines how many order parameters will be generated. Each order parameter is named based on the `op_name_base` parameter and the transformed version is named based on the `var_name_base` parameter; e.g. for `op_num` = 3, `op_name_base` = `gr`, and `var_name_base` = `phi`, three variable are created as auxvariables (`gr0`, `gr1`, and `gr2`) and three transformed nonlinear variables (`phi0`, `phi1`, `phi2`).

The following objects are created:

+Variables+

- One per variable, with a total of `op_num`

  - Transformed variable (`var_name_base`)

+AuxVariables+

- `bounds_dummy`, used to save the bounds values
- `bnds`, used to visualize grain boundaries
- One per variable, with a total of `op_num`

  - Actual order parameter value (`op_name_base`)

+Materials+

- One per variable, with a total of `op_num`

  - [LinearizedInterfaceFunction](/LinearizedInterfaceFunction.md)

+Kernels+

- One per variable, with a total of `op_num`

  - [ChangedVariableTimeDerivative](/ChangedVariableTimeDerivative.md)
  - [ACGrGrPolyLinearizedInterface](/ACGrGrPolyLinearizedInterface.md)
  - [ACInterfaceChangedVariable](/ACInterfaceChangedVariable.md)

+AuxKernels+

- [BndsCalcAux](/BndsCalcAux.md)
- One per variable, with a total of `op_num`

  - [LinearizedInterfaceAux](/LinearizedInterfaceAux.md)
  - [ConstantBoundsAux](/ConstantBoundsAux.md) (for upper bound)
  - [ConstantBoundsAux](/ConstantBoundsAux.md) (for lower bound)


## Example Input File Syntax

The `GrainGrowthLinearizedInterfaceAction` is accessed through the `Module` block, as shown below.

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/linearized_interface_action.i block=GlobalParams

!listing modules/phase_field/test/tests/grain_growth_w_linearized_interface/linearized_interface_action.i block=Modules

!syntax parameters /Modules/PhaseField/GrainGrowthLinearizedInterface/GrainGrowthLinearizedInterfaceAction

!syntax children /Modules/PhaseField/GrainGrowthLinearizedInterface/GrainGrowthLinearizedInterfaceAction

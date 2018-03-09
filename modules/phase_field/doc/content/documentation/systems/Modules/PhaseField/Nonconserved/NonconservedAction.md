# NonconservedAction

!syntax description /Modules/PhaseField/Nonconserved/NonconservedAction

This action simplifies the input file syntax for creating a nonconserved phase field variable in the phase field module. It creates the variable and all required kernels needed to solve for a nonconserved variable.

## Variables

The name of the nonconserved variable is the block name.

## Kernels

The kernels that are added are:

- [TimeDerivative](/TimeDerivative.md)
- [AllenCahn](/AllenCahn.md)
- [ACInterface](/ACInterface.md)

!syntax parameters /Modules/PhaseField/Nonconserved/NonconservedAction

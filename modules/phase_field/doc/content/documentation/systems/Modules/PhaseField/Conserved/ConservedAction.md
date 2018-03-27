# ConservedAction

!syntax description /Modules/PhaseField/Conserved/ConservedAction

This action simplifies the input file syntax for creating a conserved phase field variable in the phase field module. It creates the variables and kernels needed to solve for a conserved variable. Three solving approaches (`solve_type`) are supported:
- `direct`
- `reverse_split`
- `forward_split`

## Variables

In each approach, the name of the conserved variable is the block name.

### `direct`

The direct solve has a second order spatial derivative term in the [CHInterface](/CHInterface.md) residual, and therefore requires a higher order element. For this reason, the variable is always created to be a third-order Hermite, no matter the family and order passed into the action.

### `reverse_split`

The reverse_split adds two variables. It adds a conserved variable and a coupled variable which stores the chemical potential. Both variables have the same family and order.

### `forward_split`

The forward_split adds two variables. It adds conserved variable and a coupled variable which stores the Laplacian of the conserved variable. Both variables have the same family and order.

## Kernels

The kernels that are added depend on the solution approach:

### `direct`

- [TimeDerivative](/TimeDerivative.md)
- [CahnHilliard](/CahnHilliard.md)
- [CHInterface](/CHInterface.md)

### `reverse_split`

*Conserved variable*
- [CoupledTimeDerivative](/CoupledTimeDerivative.md)

*Coupled variable*
- [SplitCHWRes](/SplitCHWRes)
- [SplitCHParsed](/SplitCHParsed)

### `forward_split`

*Conserved variable*
- [TimeDerivative](/TimeDerivative.md)
- [MatDiffusion](/MatDiffusion.md)

*Coupled variable*
- [MatDiffusion](/MatDiffusion.md)
- [CoupledMaterialDerivative](/CoupledMaterialDerivative.md)
- [CoefReaction](/CoefReaction.md)

!syntax parameters /Modules/PhaseField/Conserved/ConservedAction

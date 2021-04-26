# VariableGradientComponent

!syntax description /AuxKernels/VariableGradientComponent

The result of this operation is stored in an auxiliary variable: `variable`. The analysis of gradients is useful to examine a solution or
the adequacy of a mesh discretization.

!alert note
AuxVariables currently do not contain the Automatic Differentiation information about the derivatives. Using this object in the solve, to compute a material property for example, will lead to an imperfect Jacobian which can impact convergence.

!alert note
VariableGradientComponent may only be applied to elemental variables, and only considers the gradient within an element. For example, the reported gradient of a `CONSTANT` `MONOMIAL` is identically zero.

## Example Input File Syntax

In this example, u is the (linear) solution of a 2D diffusion equation with no source term. We compute the two gradient
components using the AuxKernels below, which each store a gradient component in a separate auxiliary variable. We can
then examine the gradient of u, for example to decide to use a finer mesh in a given direction if the corresponding
gradient is not well resolved.

!listing /test/tests/auxkernels/grad_component/grad_component.i block=AuxKernels

!syntax parameters /AuxKernels/VariableGradientComponent

!syntax inputs /AuxKernels/VariableGradientComponent

!syntax children /AuxKernels/VariableGradientComponent

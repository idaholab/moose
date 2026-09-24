# SumOperatorExtension

!if! function=hasCapability('mfem')

## Summary

This operator forms $ \alpha A + \beta B $, where $A$ is the gradient of a partially-assembled nonlinear form,
and $B$ is the constrained linear system operator.

The order in which these two are passed to the constructor is crucial, since we call `B->AssembleDiagonal`
directly, which would fail for the nonlinear gradient.

The reason for this class is because `mfem::SumOperator` does not have the `AssembleDiagonal` method. The one
we've written is analogous to `mfem::PABilinearFormExtension::AssembleDiagonal`, except we build the diagonal
using the nonlinear form's `GetDNFI()/GetBNFI()` methods.

We make the choice here to have the diagonal value on essential rows to be 1. To achieve this, we make sure
that the `mfem::Operator::DiagonalPolicy` for the gradient of the nonlinear form is 0, and for the constrained
linear system operator it's 1.

The only place that it's used is [EquationSystem.md].

!if-end!

!else
!include mfem/mfem_warning.md

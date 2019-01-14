# KernelValue

Derive from this base class if your residual is of the form

$$
(\dots,\psi_i)
$$

i.e. if the test function $\psi_i$ (`_test[_i][_qp]`) can be factored out.

## Derived classes

Override

- `precomputeQpResidual()` instead of `computeQpResidual()` (do not multiply by `_test[_i][_qp]`)
- `precomputeQpJacobian()` instead of `computeQpJacobian()` (do not multiply by `_test[_i][_qp]`)
- `computeOffDiagJacobian` still has to be implemented as if deriving from `Kernel`.

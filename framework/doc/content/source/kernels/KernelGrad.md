# KernelGrad

Derive from this base class if your residual is of the form

\begin{equation}
  (\dots,\nabla\psi_i)
\end{equation}

i.e. if the test function $\nabla\psi_i$ (`_grad_test[_i][_qp]`) can be factored out.

## Derived classes

Override

- `precomputeQpResidual()` instead of `computeQpResidual()` (do not multiply by `_grad_test[_i][_qp]`).
- `precomputeQpJacobian()` instead of `computeQpJacobian()` (do not multiply by `_grad_test[_i][_qp]`)
- `computeOffDiagJacobian` still has to be implemented as if deriving from `Kernel`.

Note that `precomputeQpResidual()` and `precomputeQpJacobian()` return a `RealGradient` type.

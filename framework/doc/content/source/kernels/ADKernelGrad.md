# ADKernelGrad

## Description

This is an optimized [Kernel](/Kernel.md) base class for residuals which allow
the gradient of the test function $\nabla\psi_i$ (`_grad_test[_i][_qp]`) to be
factored out (see [KernelGrad](/KernelGrad.md)).

$$
(\dots,\nabla\psi_i)
$$

The Jacobian in `ADKernelGrad` is computed using forward automatic
differentiation.

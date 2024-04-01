# ADKernelValue

## Description

This is an optimized [Kernel](/Kernel.md) base class for residuals which allow
the test function  $\psi_i$ (`_test[_i][_qp]`) to be factored out (see
[KernelValue](/KernelValue.md)).

\begin{equation}
  (\dots,\psi_i)
\end{equation}

The Jacobian in `ADKernelValue` is computed using forward automatic
differentiation.

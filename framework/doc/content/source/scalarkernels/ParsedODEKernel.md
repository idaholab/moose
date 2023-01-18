# ParsedODEKernel

This scalar kernel adds a source term $s(u, \mathbf{v}, \mathbf{p})$:
\begin{equation}
  \frac{d u}{d t} = s(u, \mathbf{v}, \mathbf{p}) \,,
\end{equation}
where $u$ is the variable the source acts upon, $\mathbf{v}$ are other
scalar variables, and $\mathbf{p}$ are post-processor values.

The parameter `expression` is a string expression for the source term $s(u, \mathbf{v}, \mathbf{p})$
+as it appears on the left-hand-side of the equation+; thus it represents $-s(u, \mathbf{v}, \mathbf{p})$.
The expression may use the following quantities:

- the name of the scalar variable upon which the kernel acts,
- the names of any scalar variables specified in the `coupled_variables` parameter,
- the names of any post-processors specified in the `postprocessors` parameter, and
- the names supplied in the `constant_names` parameter, defined to have the
  values provided by the `constant_expressions` parameters.

Currently, the function expression cannot be a function of time.

!listing examples/ex18_scalar_kernel/ex18_parsed.i block=ScalarKernels

!syntax parameters /ScalarKernels/ParsedODEKernel

!syntax inputs /ScalarKernels/ParsedODEKernel

!syntax children /ScalarKernels/ParsedODEKernel

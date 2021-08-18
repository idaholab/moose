# ScalarKernels System

Scalar kernels are used to define systems of ordinary differential equations (ODEs),
which lack spatial derivatives. These are used in initial value problems, with
time as the independent variable:
\begin{equation}
\begin{split}
  &\frac{d u}{d t} = f(u, t) \qquad t \geq 0\\
  &u(0) = u_0\\
\end{split}
\end{equation}
where $u(t)$ is the dependent variable, $f(u, t)$ is the steady-state residual
function, and $u_0$ is the initial value.

## Automatic Differentiation

Scalar kernels have the ability to be implemented with
[automatic differentiation (AD)](automatic_differentiation/index.md).
While AD is not necessary for systems of ordinary differential equations (ODEs)
involving only scalar variables (due to the exact Jacobians offered by
[ParsedODEKernel.md], for example), ODEs involving contributions from field
variables greatly benefit from AD. For example, an elemental user object may
compute an `ADReal` value from field variable(s) on a domain, which then may
be used in a scalar equation.

To create an AD scalar kernel, derive from `ADScalarKernel` and implement the
method `computeQpResidual()`.

!alert warning title=AD global indexing required
`ADScalarKernel` only works with MOOSE configured with global AD indexing (the default).

!alert warning title=Using values computed by user objects
As a caution, if using user objects to compute
`ADReal` values, be sure to execute those user objects on `NONLINEAR` to
ensure the derivatives in the `ADReal` value are populated.

!syntax list /ScalarKernels objects=True actions=False subsystems=False

!syntax list /ScalarKernels objects=False actions=False subsystems=True

!syntax list /ScalarKernels objects=False actions=True subsystems=False

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

Similar to the [Kernel](syntax/Kernels/index.md) class, in a `ScalarKernel` subclass the
`computeResidual()` function +must+ be overridden.  This is where you implement
your ODE weak form terms.  For non-AD objects the following member function can
optionally be overridden:

- `computeJacobian()`
- `computeOffDiagJacobianScalar()`

## Coupling with Spatial Variables id=couple-spatial

For systems of coupled partial differential equations (PDEs) and ODEs, typically
integration over domains and manifolds are needed within the coupling terms of the
weak form. Since the `ScalarKernel` class does not provide for integration over
elements or faces, there are two options for performing needed integration using
other object classes:

1. Compute integrals for residual and diagonal Jacobian entries of the scalar
   variable within a `UserObject` and connect that value into the `ScalarKernel` object.
   Cross Jacobian terms that couple the scalar and spatial variables need to be handled
   by overridden assembly routines that access upper and lower triangular blocks of the
   Jacobian concurrently. An example of this approach is provided in the
   [AverageValueConstraint.md] and the [ScalarLagrangeMultiplier.md] objects, respectively.

2. Compute all integrals for the residual and Jacobian entries for the spatial and
   scalar variables using scalar augmentation classes that derive from the
   respective spatial variable residual object class. This approach is described below.

The principal purpose of these scalar augmentation classes is to add standard
quadrature loops and assembly routines to handle the contributions from a single added
scalar variable to that object, including the entire row of the Jacobian. 
This scalar variable is referred to as the "focus" scalar variable of that object.
Lists of interfaces for
the quadrature point routines are given in the links below. This system is currently being
developed and will extend to the other residual objects.

| Object | Scalar Augmentation Class | Example Derived Class |
| :- | :- | :- |
| Kernel\\ +ADKernel+ | [`KernelScalarBase`](source/kernels/KernelScalarBase.md) | [`ScalarLMKernel`](source/kernels/ScalarLMKernel.md) |
| IntegratedBC | Under Development |  |
| InterfaceKernel | Under Development |  |
| DGKernel | Under Development |  |
| MortarConstraint\\ +ADMortarConstraint+ | [`MortarScalarBase`](source/constraints/MortarScalarBase.md) | [`PeriodicSegmentalConstraint`](source/constraints/PeriodicSegmentalConstraint.md) |

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

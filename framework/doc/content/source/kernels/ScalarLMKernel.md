# ScalarLMKernel

This `Kernel` demonstrates the usage of the scalar augmentation class described in [KernelScalarBase.md].
This single kernel is an alternative to the combination of [ScalarLagrangeMultiplier.md],
[AverageValueConstraint](source/scalarkernels/AverageValueConstraint.md), and an
[Elemental Integral Postprocessor](source/postprocessors/ElementIntegralVariablePostprocessor.md).
All terms from the spatial and scalar variables are handled by this object.

This Kernel enforces the constraint of

\begin{equation}
 \int_{\Omega} \phi = V_0
\end{equation}

where $V_0$ is a given constant, using a Lagrange multiplier approach. Since this is
a single constraint, a single [scalar variable](source/variables/MooseVariableScalar.md) $\lambda$ is required, as shown below.

```
[Variables]
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]
```

The residual of $\phi$ related to the Lagrange multiplier is given as:

\begin{equation}
  F^{(\phi)}_i \equiv \lambda^h \int_{\Omega} \varphi_i \;\text{d}\Omega \label{eq:eq1}
\end{equation}

The residual of the Lagrange multiplier $\lambda$ is given as:

\begin{equation}
  F^{(\lambda)} \equiv \int_{\Omega} \phi^h \;\text{d}\Omega - V_0 = 0 \label{eq:eq2}
\end{equation}

This Kernel implements the residual and Jacobian contributions to the spatial variable $\phi$ equation
from the $\lambda$ in [eq:eq1].
Also, it implements the residual and Jacobian contributions to the scalar Lagrange multiplier
$\lambda$ constraint equation from the spatial variable $\phi$ in [eq:eq2].

So that the input syntax matches with [ScalarLagrangeMultiplier.md], a `Postprocessor` is required
that computes the total volume of the domain, assigned to the parameter `pp_name`.

Currently, a [NullScalarKernel](source/scalarkernels/NullScalarKernel.md) is required to activate the dependency of the scalar variable within the block or subdomain of this object. See one of the example files listed below.

The detailed description of the derivation of the weak form
can be found at [scalar_constraint_kernel](https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf).

!syntax parameters /Kernels/ScalarLMKernel

!syntax inputs /Kernels/ScalarLMKernel

!syntax children /Kernels/ScalarLMKernel

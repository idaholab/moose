# FunctorKernel

This [/Kernel.md] calculates the residual based on a [functor](Functors/index.md).
The form of the contribution is specified via the `mode` parameter.

If the ‘add’ option is selected for `mode`, this kernel simply adds the term
from the [functor](Functors/index.md) value.

If, on the other hand, the ‘target’ option is selected for `mode`, this kernel
sets the kernel variable $u$ (`variable`) weakly enforced to the value of a
functor $p$ (`functor`).

!equation
\left(\pm(p-u),\psi\right)

!alert note
This kernel only imposes the equality between the variable and the functor
(i.e. `mode` set to ‘target’) when it is the only kernel in that variable's
equation on the specified subdomains.

The parameter [!param](/Kernels/FunctorKernel/functor_on_rhs) determines the
sign applied to the term. Since by convention in MOOSE, all terms are moved
to the left hand side of the equation, a value of `true` for this parameter
applies a sign of -1, and a value of `false` applies a sign of +1.

!syntax parameters /Kernels/FunctorKernel

!syntax inputs /Kernels/FunctorKernel

!syntax children /Kernels/FunctorKernel

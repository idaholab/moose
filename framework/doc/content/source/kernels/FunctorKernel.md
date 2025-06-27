# FunctorKernel

This [/Kernel.md] adds a term from a [functor](Functors/index.md) value.
The parameter [!param](/Kernels/FunctorKernel/functor_on_rhs) determines the
sign applied to the term. Since by convention in MOOSE, all terms are moved
to the left hand side of the equation, a value of `true` for this parameter
applies a sign of -1, and a value of `false` applies a sign of +1.

!syntax parameters /Kernels/FunctorKernel

!syntax inputs /Kernels/FunctorKernel

!syntax children /Kernels/FunctorKernel

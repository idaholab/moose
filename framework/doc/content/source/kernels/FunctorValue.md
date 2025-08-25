# FunctorValue

!syntax description /Kernels/FunctorValue

This kernel sets the kernel variable $u$ (`variable`) weekly enforced to the
value of a functor $p$ (`functor`).

!equation
\left(\pm(p-u),\psi\right)

The $\pm$ sign is controlled by the `positive` boolean parameter.

!alert note
This kernel does not impose equality if other kernels are used. This is
unless a penalty constraint would be imposed.

!syntax parameters /Kernels/FunctorValue

!syntax inputs /Kernels/FunctorValue

!syntax children /Kernels/FunctorValue

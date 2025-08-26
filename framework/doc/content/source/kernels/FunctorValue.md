# FunctorValue

!syntax description /Kernels/FunctorValue

This kernel sets the kernel variable $u$ (`variable`) weekly enforced to the
value of a functor $p$ (`functor`).

!equation
\left(\pm(p-u),\psi\right)

The $\pm$ sign is controlled by the `positive` boolean parameter.

!alert note
This kernel only imposes the equality between the variable and the functor when it is the only kernel in that variable's equation on the specified subdomains.

!syntax parameters /Kernels/FunctorValue

!syntax inputs /Kernels/FunctorValue

!syntax children /Kernels/FunctorValue

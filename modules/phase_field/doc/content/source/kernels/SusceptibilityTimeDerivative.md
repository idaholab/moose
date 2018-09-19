# SusceptibilityTimeDerivative

!syntax description /Kernels/SusceptibilityTimeDerivative

Implements

\begin{equation}
F(u,a,b,\dots)\cdot\frac{\partial u}{\partial t},
\end{equation}

where $F$ (`f_name`) is a [FunctionMaterial](/FunctionMaterials.md) providing derivatives
(for example defined using the [DerivativeParsedMaterial](/DerivativeParsedMaterial.md)),
$u$ is the kernel variable the time derivative is taken of, and $a, b, \dots$ (`args`)
are further arguments of the susceptibility function $F$ which should contribute to
off-diagonal Jacobian entries.

See also [TimeDerivative](/TimeDerivative.md).

!syntax parameters /Kernels/SusceptibilityTimeDerivative

!syntax inputs /Kernels/SusceptibilityTimeDerivative

!syntax children /Kernels/SusceptibilityTimeDerivative

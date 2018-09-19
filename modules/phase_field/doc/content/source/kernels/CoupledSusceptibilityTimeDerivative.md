# CoupledSusceptibilityTimeDerivative

!syntax description /Kernels/CoupledSusceptibilityTimeDerivative

Implements

\begin{equation}
F(u,v,a,b,\dots)\cdot\frac{\partial v}{\partial t},
\end{equation}

where $F$ (`f_name`) is a [FunctionMaterial](/FunctionMaterials.md) providing derivatives
(for example defined using the [DerivativeParsedMaterial](/DerivativeParsedMaterial.md)),
$u$ is the variable the kernel is acting on, $v$ (`v`) is the coupled variable the time
derivative is taken of, and $a, b, \dots$ (`args`) are further arguments of the susceptibility
function $F$ which should contribute to off-diagonal Jacobian entries.

See also [CoupledTimeDerivative](/CoupledTimeDerivative.md).s

!syntax parameters /Kernels/CoupledSusceptibilityTimeDerivative

!syntax inputs /Kernels/CoupledSusceptibilityTimeDerivative

!syntax children /Kernels/CoupledSusceptibilityTimeDerivative

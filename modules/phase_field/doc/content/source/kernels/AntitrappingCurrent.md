# AntitrappingCurrent

!syntax description /Kernels/AntitrappingCurrent

Implements the antitrapping current term corresponding to alloy solidification [!cite](plapp_unified_2011). The weak form is

\begin{equation}
\left( F(u,v,a,b,\dots) \frac{\nabla v}{|\nabla v|} \frac{\partial v}{\partial t}, \nabla \psi \right),
\end{equation}

where $F$ (`f_name`) is a [FunctionMaterial](/FunctionMaterials.md) providing derivatives, $u$ is the variable the kernel is acting on, $v$ (`v`) is the coupled variable the time derivative is taken of, and $a, b, \dots$ (`args`) are further arguments of the susceptibility function $F$ that contributes to off-diagonal Jacobian entries, and $\psi$ is the test function. Here, $\frac{\nabla v}{|\nabla v|}$ represents the unit normal to the interface.

See also [CoupledSusceptibilityTimeDerivative](/CoupledSusceptibilityTimeDerivative.md)

!syntax parameters /Kernels/AntitrappingCurrent

!syntax inputs /Kernels/AntitrappingCurrent

!syntax children /Kernels/AntitrappingCurrent

!bibtex bibliography

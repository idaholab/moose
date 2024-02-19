# CoupledSwitchingTimeDerivative / ADCoupledSwitchingTimeDerivative

!syntax description /Kernels/CoupledSwitchingTimeDerivative

`CoupledSwitchingTimeDerivative` (and its automatic differentiation version, `ADCoupledSwitchingTimeDerivative`) is a coupled time derivative Kernel that multiplies the time derivative term by the following
\begin{equation}
\frac{\partial h_a}{\partial\eta_{ai}} F_a + \frac{\partial h_b}{\partial\eta_{ai}} F_b + \dots,
\end{equation}
where $a,b,\dots$ are the phases, $h_a, h_b,\dots$ are the switching functions,
$\eta_{ai}$ is the order parameter for the phase/grain that is the nonlinear variable,
and $F_a, F_b,\dots$ are the free energies or grand potentials.

See also [CoupledTimeDerivative](/CoupledTimeDerivative.md).

!syntax parameters /Kernels/CoupledSwitchingTimeDerivative

!syntax inputs /Kernels/CoupledSwitchingTimeDerivative

!syntax children /Kernels/CoupledSwitchingTimeDerivative

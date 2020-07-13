# ADCoupledSwitchingTimeDerivative

!syntax description /Kernels/ADCoupledSwitchingTimeDerivative

This kernel adds a contribution
\begin{equation}
\left( \frac{\partial h_a}{\partial \eta_{ai}} F_a +
          \frac{\partial h_b}{\partial \eta_{ai}} F_b + ... \right)  \frac{\partial \eta_{ai}}{\partial t}
\end{equation}
where $a,b,\dots$ are the phases, $h_a, h_b,\dots$ are the switching functions that can be obtained from [ADSwitchingFunctionMultiPhaseMaterial](/ADSwitchingFunctionMultiPhaseMaterial.md),
$\eta_{ai}$ is the order parameter that is the nonlinear variable, $t$ is time,
and $F_a, F_b,\dots$ are functions for each phase. For the grand-potential
model susceptibility equation, $F_a$ etc. represents the phase densities.


!syntax parameters /Kernels/ADCoupledSwitchingTimeDerivative

!syntax inputs /Kernels/ADCoupledSwitchingTimeDerivative

!syntax children /Kernels/ADCoupledSwitchingTimeDerivative

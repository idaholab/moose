# ADSusceptibilityTimeDerivative

!syntax description /Kernels/ADSusceptibilityTimeDerivative

This kernel is the AD version of the [SusceptibilityTimeDerivative](\SusceptibilityTimeDerivative). This calculates the time derivative for a variable multiplied by a generalized susceptibility

\begin{equation}
F(u,a,b,\dots)\cdot\frac{\partial u}{\partial t},
\end{equation}

where $F$ (`f_name`) is the susceptibility function,
$u$ is the kernel variable the time derivative is taken of, and $a, b, \dots$ are other variables that the susceptibility function $F$ depends on.

!syntax parameters /Kernels/ADSusceptibilityTimeDerivative

!syntax inputs /Kernels/ADSusceptibilityTimeDerivative

!syntax children /Kernels/ADSusceptibilityTimeDerivative

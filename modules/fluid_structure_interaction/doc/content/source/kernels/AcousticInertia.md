# Acoustic inertia

!syntax description /Kernels/AcousticInertia

## Description

This kernel computes the residual contribution from the double time derivative of
the pressure term:

\begin{equation}
    \label{eqn:sample}
    \frac{1}{c_o^2} \frac{\partial^2 p}{\partial t^2}
\end{equation}

This kernel is similar to the [InertialForce](/InertialForce.md) kernel. Also, this kernel is part of the fluid-structure interaction codes. Please refer to [fluid-structure interaction using acoustics](/fsi_acoustics.md) for the theoretical details.

!alert note title=Units for defining the mass and weight quantities
In [eqn:sample], the inverse of square of the fluid speed of sound is taken.
For water, in standard SI units (i.e., m, s, kg, and N), this value is of the order
$4\times 10^{-7}$ which is very small. If the fluid-structure interaction problem
is run in standard SI units, the presence of $\frac{1}{c_o^2}$ results in bad scaling
of the Jacobian matrices and hence poor convergence. To avoid this convergence issue,
it is recommended that Giga Newtons be used instead of Newtons, Giga kg be used instead of kg.

!syntax parameters /Kernels/AcousticInertia

!syntax inputs /Kernels/AcousticInertia

!syntax children /Kernels/AcousticInertia

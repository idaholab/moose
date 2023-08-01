# ADShaftComponentTorqueScalarKernel

!syntax description /ScalarKernels/ADShaftComponentTorqueScalarKernel

This scalar kernel adds the value of the shaft torque $\tau(t)$ as a source term for an ODE:
\begin{equation}
  \frac{du}{dt} = \tau(t) \,
\end{equation}

!alert note
In THM, most kernels are added automatically by components. This scalar kernel is created by the
[Shaft.md] component.

!alert note
This is the [automatic differentiation (AD)](automatic_differentiation/index.md) version of [ShaftComponentTorqueScalarKernel.md].
AD is used in THM to compute numerically exact Jacobians.

!syntax parameters /ScalarKernels/ADShaftComponentTorqueScalarKernel

!syntax inputs /ScalarKernels/ADShaftComponentTorqueScalarKernel

!syntax children /ScalarKernels/ADShaftComponentTorqueScalarKernel

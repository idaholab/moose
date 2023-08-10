# ShaftComponentTorqueScalarKernel

!syntax description /ScalarKernels/ShaftComponentTorqueScalarKernel

This scalar kernel adds the value of the shaft torque $\tau(t)$ as a source term for an ODE:
\begin{equation}
  \frac{du}{dt} = \tau(t) \,
\end{equation}

!alert note
In THM, most kernels are added automatically by components. This scalar kernel is created by the
[Shaft.md] component.

!syntax parameters /ScalarKernels/ShaftComponentTorqueScalarKernel

!syntax inputs /ScalarKernels/ShaftComponentTorqueScalarKernel

!syntax children /ScalarKernels/ShaftComponentTorqueScalarKernel

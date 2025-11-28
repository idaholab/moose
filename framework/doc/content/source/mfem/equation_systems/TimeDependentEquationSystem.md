# TimeDependentEquationSystem

!if! function=hasCapability('mfem')

For transient problems, time derivatives $\dot{u}$ of the trial variables $u$ will also be
present in the weak form.

!equation
\left(\mathcal{T}(\dot{u}), v\right)_{\Omega} + {\left(\mathcal{L}(u), v\right)_{\Omega}
=\left(f,v\right)_{\Omega}\,\,\,\forall v \in V}

Contributions to $\left(\mathcal{T}(\phi_j), \varphi_i\right)_{\Omega}$ are given by
time derivative kernels such as
 [MFEMTimeDerivativeMassKernel](source/mfem/kernels/MFEMTimeDerivativeMassKernel.md)

Currently, transient problems in MOOSE's wrapping of MFEM are solved using an implicit backwards
Euler method.
Denoting $u_n=u(t+\delta t)$, $u_{n-1}=u(t)$ and approximating
$u(t+\delta t) \approx u(t) + \delta t \dot{u}(t+\delta t)$, we have

!equation
\left([\mathcal{T}+\delta t\mathcal{L}](u_n), v\right)_{\Omega}
=\left([\delta t f + \mathcal{T}(u_{n-1})],v\right)_{\Omega}\,\,\,\forall v \in V

!if-end!

!else
!include mfem/mfem_warning.md

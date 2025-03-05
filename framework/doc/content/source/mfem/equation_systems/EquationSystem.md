# EquationSystem

The EquationSystem is responsible for defining and assembling the weak form of the PDE into an
[`mfem::Operator`](https://docs.mfem.org/html/classmfem_1_1Operator.html) used to solve an iteration
of the FE problem. This operator is passed to an
[`mfem::NewtonSolver`](https://docs.mfem.org/html/classmfem_1_1NewtonSolver.html) in a
[ProblemOperator](source/mfem/problem_operators/ProblemOperator.md), which handles the update of the
state of all variables (including any required nonlinear iterations).

!equation
{\left(\mathcal{L}(u), v\right)_{\Omega}=\left(f,v\right)_{\Omega}\,\,\,\forall v \in V}

Discretizing the trial variable $u$ by $u\approx u_h = \sum_i u_i \phi_i(\vec x)$,
and approximating the test space $V$ by a finite dimensional subspace spanned by
the basis $\{\varphi_i\}$, the weak form becomes

!equation
{\sum_{j}u_j\left(\mathcal{L}(\phi_j),\varphi_i\right)_{\Omega}=\left(f,\varphi_i\right)_{\Omega}}

after variation over the test variable $v$. This can be expressed in matrix form by
defining $A_{ij} = \left(\mathcal{L}(\phi_j), \varphi_i\right)_{\Omega}$ and
${b_i=\left(f,\varphi_i\right)_{\Omega}}$.

!equation
{\sum_j A_{ij} u_j = b_i}

[MFEMKernels](source/mfem/kernels/MFEMKernel.md) contribute domain integrators to the weak form, and
[MFEMIntegratedBCs](source/mfem/bcs/MFEMIntegratedBC.md) contribute boundary integrators to the weak
form. [`mfem::BilinearFormIntegrators`](https://mfem.org/bilininteg/) add contributions to
$A_{ij}(\varphi_i, \phi_j)$ and [`mfem::LinearFormIntegrators`](https://mfem.org/lininteg/) add
contributions to $b_i(\varphi_i)$ when assembled.

## TimeDependentEquationSystem

For transient problems, time derivatives $\dot{u}$ of the trial variables $u$ will also be
present in the weak form.

!equation
\left(\mathcal{T}(\dot{u}), v\right)_{\Omega} + {\left(\mathcal{L}(u), v\right)_{\Omega}
=\left(f,v\right)_{\Omega}\,\,\,\forall v \in V}

Contributions to $\left(\mathcal{T}(\phi_j), \varphi_i\right)_{\Omega}$ are given by
time derivative kernels such as
 [MFEMTimeDerivativeMassKernel](source/mfem/kernels/MFEMTimeDerivativeMassKernel.md)

Currently, transient problems in MOOSE's wrapping of MFEM are solved using an implicit backwards Euler method.
Denoting $u_n=u(t+\delta t)$, $u_{n-1}=u(t)$ and approximating
$u(t+\delta t) \approx u(t) + \delta t \dot{u}(t+\delta t)$, we have

!equation
\left([\mathcal{T}+\delta t\mathcal{L}](\dot{u}_n), v\right)_{\Omega}
=\left([f-\mathcal{L}(u_{n-1})],v\right)_{\Omega}\,\,\,\forall v \in V

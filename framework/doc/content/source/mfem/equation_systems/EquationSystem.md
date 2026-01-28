# EquationSystem

!if! function=hasCapability('mfem')

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

[MFEMKernels](source/mfem/kernels/MFEMKernel.md) can also contribute to domain integrators for 
non-linear actions. This allows to form the residual $\mathcal{L}(u)$ for non-linear Newton's 
methood as shown below

!equation
{\mathbf{J}\left(\vec{u}_n\right) \delta \vec{u}_{n+1}=-\vec{R}\left(\vec{u}_n\right)}

!equation
{\vec{u}_{n+1}=\vec{u}_n+\delta \vec{u}_{n+1}}

where $\mathbf{J}$ is the Jacobian, and $\delta \vec{u}$ is the incremental solution.

!if-end!

!else
!include mfem/mfem_warning.md

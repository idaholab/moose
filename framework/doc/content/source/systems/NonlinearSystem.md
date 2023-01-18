# NonlinearSystem

The NonlinearSystem object holds the equation system created by the normal FEM process
(e.g. the Matrix and RHS vector) to be solved. Normally MOOSE uses PETSc to store and
solve this system. This object is where you will find the callback routines used
by the PETSc solvers.

- [#newtons_method]
- [#jacobian_definition]
- [#linear_methods]
- [#augmenting_sparsity]

You may find some additional documentation relevant to both `NonlinearSystem`
and `NonlinearEigenSystem` in [NonlinearSystemBase.md].

## Solving Non-linear Systems id=newtons_method

Application of the finite element method converts PDE(s) into a system of
nonlinear equations, $R_i(u_h)=0, \qquad i=1,\ldots, N$
  to solve for the coefficients $u_j, j=1,\dots,N$.

- Newton's method has good convergence properties, we use it to solve this system of nonlinear equations.
- Newton's method is a "root finding" method: it finds zeros of nonlinear equations.
- Newton's Method in "Update Form" for finding roots of the scalar equation
  $\begin{array}{rl}f(x)&=0, f(x): \mathbb{R} &\rightarrow \mathbb{R}\textrm{ is given by}:\\
  f'(x_n) \delta x_{n+1} &= -f(x_n) \\
  x_{n+1} &= x_n + \delta x_{n+1}\end{array}$
- We don't have just one scalar equation: we have a system of nonlinear equations.
- This leads to the following form of Newton's Method:

    $\begin{aligned}
    \mathbf{J}(\vec{u}_n) \delta\vec{u}_{n+1} &= -\vec{R}(\vec{u}_n) \\
    \vec{u}_{n+1} &= \vec{u}_n + \delta\vec{u}_{n+1}\end{aligned}$

- Where $\mathbf{J}(\vec{u}_n)$ is the Jacobian matrix evaluated at the current iterate:
    $J_{ij}(\vec{u}_n) = \frac{\partial R_i(\vec{u}_n)}{\partial u_j}$

- Note that:
    $\frac{\partial u_h}{\partial u_j} =
      \sum_k\frac{\partial }{\partial u_j}\left(u_k \phi_k\right) = \phi_j
    \qquad
    \frac{\partial \left(\nabla u_h\right)}{\partial u_j} =
      \sum_k \frac{\partial }{\partial u_j}\left(u_k \nabla \phi_k\right) = \nabla \phi_j$


## Jacobian Definition id=jacobian_definition

An efficient Newton solve, e.g. one that requires few "nonlinear" iterations,
requires an accurate Jacobian matrix or an accurate approximation of its action
on a vector. When no explicit matrix is formed for the Jacobian and only its
action on a vector is computed, the algorithm is commonly referred to as
matrix-free (PETSc jargon) or [Jacobian-free](#JFNK) (MOOSE jargon). The default
solve algorithm in MOOSE is `PJFNK`, or Preconditioned Jacobian-Free
Newton-Krylov. "Krylov" refers to the *linear* solution algorithm used to solve
each nonlinear iteration of the Newton algorithm. For more information on
solving linear systems, please see [#linear_methods]. Even if a Jacobian-free
nonlinear algorithm is chosen, typically a good preconditioning matrix is still
needed. Building the matrix can be accomplished
[automatically, using automatic differentiation](#AD) and/or [manually](#hand_coded_jac).

### Automatic Differentiation id=AD

One can elect to sacrifice some computing speed and calculate Jacobians
automatically using
[automatic differentiation (AD)](https://en.wikipedia.org/wiki/Automatic_differentiation). MOOSE
employs the `DualNumber` class from the
[MetaPhysicL](https://github.com/roystgnr/MetaPhysicL) package in order to
enable AD. If the application developer wants to make use of AD, they should
inherit from `ADKernel` as opposed to `Kernel`. Additionally, when coupling in
variables, the `adCoupled*` methods should be used. For example, to retrieve a
coupled value, instead of using `coupledValue("v")` in the `ADKernel`
constructor, `adCoupledValue("v")` should be used. `adCoupledGradient` should
replace `coupledGradient`, etc. An example of coupling in an AD variable can be found in
[`ADCoupledConvection.C`](test/src/kernels/ADCoupledConvection.C) and
[`ADCoupledConvection.h`](test/include/kernels/ADCoupledConvection.h). Moreover,
material properties that may depend on the nonlinear variables should be
retrieved using `getADMaterialProperty` instead of `getMaterialProperty`. They
should be declared in materials using `declareADProperty`. Example AD material
source and header files can be found
[here](test/src/materials/ADCoupledMaterial.C) and
[here](test/include/materials/ADCoupledMaterial.h); example kernel source and
header files that use AD material properties can be found
[here](test/src/kernels/ADMatDiffusionTest.C) and
[here](test/include/kernels/ADMatDiffusionTest.h). The object central to AD
computing objects is `ADReal` which is defined in [`MooseTypes`](/MooseTypes.md).

### Traditional Hand-coded Jacobians id=hand_coded_jac

Finite element shape functions are introduced in the documentation section
[FEProblemBase.md#shape_functions]. There we outline
how our primary variables are summations of those shape functions multiplied by
constant coefficients which are our degrees of freedom. At the end of [#newtons_method]
we gave an explicit illustration of how the derivative of a variable `u` with
respect to its jth degree of freedom ($u_j$) is equal to the jth shape function
$\phi_j$. Similarly the derivative of $\nabla u$ with respect to $u_j$ is
equal to $\nabla \phi_j$. The code expression  `_phi[_j][_qp]` represents
$\frac{\partial u}{\partial u_j}$ in any MOOSE framework residual and Jacobian
computing objects such as kernels and boundary conditions.

Any MOOSE kernel may have an arbitrary number of variables coupled into it. If
these coupled variables use the same shape function family and order, then their
associated $\phi_j$s will be equivalent. However, if `u` and `v` use different
shape functions then $\phi_{j,u} \ne \phi_{j,v}$. As a developer, however, you
do not ***in most cases*** have to worry about these differences in $\phi$. MOOSE automatically
updates the object member variable `_phi` to use the shape functions of the
variable for whom the Jacobian is currently being computed. ***However***, if
the primary variable `u` is a scalar-valued (single-component) finite element
variable and the coupled variable `v` is a vector-valued (multi-component)
finite element variable (or visa versa), then you must introduce an additional
member variable to represent the shape functions of the vector-valued
(scalar-valued) variable. The name of this variable is up to the developer, but
we suggest perhaps a `_standard_` prefix for scalar valued finite-element
variables and `_vector_` for vector valued finite-element variables. The
`_standard_` prefix is suggested over `_scalar_` so as not to be confused with a
`MooseVariableScalar`, which only has a single value over the entire spatial
domain. An example constructor for a standard kernel that couples in a
vector-valued FE variable is shown below:

```
EFieldAdvection::EFieldAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _efield_id(coupled("efield")),
    _efield(coupledVectorValue("efield")),
    _efield_var(*getVectorVar("efield", 0)),
    _vector_phi(_assembly.phi(_efield_var)),
    _mobility(getParam<Real>("mobility"))
{
}
```

The associated declarations are:

```
  const unsigned int _efield_id;
  const VectorVariableValue & _efield;
  VectorMooseVariable & _efield_var;
  const VectorVariablePhiValue & _vector_phi;
  const Real _mobility;
  Real _sgn;
```

Residual, on-diagonal, and off-diagonal methods are respectively

```
Real
EFieldAdvection::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _sgn * _mobility * _efield[_qp] * _u[_qp];
}
```

and

```
Real
EFieldAdvection::computeQpJacobian()
{
  return -_grad_test[_i][_qp] * _sgn * _mobility * _efield[_qp] * _phi[_j][_qp];
}
```

and

```
Real
EFieldAdvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _efield_id)
    return -_grad_test[_i][_qp] * _sgn * _mobility * _vector_phi[_j][_qp] * _u[_qp];
  else
    return 0;
}

```
An example constructor for a vector kernel that couples in a
scalar-valued FE variable is shown below:

```
VectorCoupledGradientTimeDerivative::VectorCoupledGradientTimeDerivative(
    const InputParameters & parameters)
  : VectorKernel(parameters),
    _grad_v_dot(coupledGradientDot("v")),
    _d_grad_v_dot_dv(coupledDotDu("v")),
    _v_id(coupled("v")),
    _v_var(*getVar("v", 0)),
    _standard_grad_phi(_assembly.gradPhi(_v_var))
{
}

```
The associated declarations are:

```
  const VariableGradient & _grad_v_dot;
  const VariableValue & _d_grad_v_dot_dv;
  const unsigned _v_id;
  MooseVariable & _v_var;
  const VariablePhiGradient & _standard_grad_phi;
```

Residual and off-diagonal Jacobian methods are respectively:

```
Real
VectorCoupledGradientTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _grad_v_dot[_qp];
}
```

and

```
Real
VectorCoupledGradientTimeDerivative::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _v_id)
    return _test[_i][_qp] * _d_grad_v_dot_dv[_qp] * _standard_grad_phi[_j][_qp];

  else
    return 0.;
}
```

!alert note title=Flexibility
Note that only one member is needed to represent shape functions for standard
    `MooseVariable`s and `VectorMooseVariable`s. For example, if the vector-variables
    `v` and `w` are coupled into a standard kernel for `u`, only a single
    `_vector_phi` member needs to be added; there is not need for both a
    `_v_phi` and `_w_phi`. `_vector_phi` will be automatically updated to
    represent the shape functions for whichever vector variable the Jacobian is
    being computed for.

### Newton for a Simple Equation id=simple_newton

- Consider the convection-diffusion equation with nonlinear $k$, $\vec{\beta}$, and $f$:
    $\begin{aligned}- \nabla\cdot k\nabla u + \vec{\beta} \cdot \nabla u = f\end{aligned}$

- The $i^{th}$ component of the residual vector is:
    $\begin{aligned}
    R_i(u_h) = \left(\nabla\psi_i, k\nabla u_h \right) - \langle\psi_i, k\nabla u_h\cdot \hat{n} \rangle +
    \left(\psi_i, \vec{\beta} \cdot \nabla u_h\right) - \left(\psi_i, f\right)\end{aligned}$


- Using the previously-defined rules for $\frac{\partial u_h}{\partial u_j}$ and $\frac{\partial \left(\nabla u_h\right)}{\partial u_j}$, the $(i,j)$ entry of the Jacobian is then:

$\begin{aligned} J_{ij}(u_h) &= \left(\nabla\psi_i, \frac{\partial k}{\partial u_j}\nabla u_h \right) + \left(\nabla\psi_i, k \nabla \phi_j \right) - \left \langle\psi_i, \frac{\partial k}{\partial u_j}\nabla u_h\cdot \hat{n} \right\rangle \\&- \left \langle\psi_i, k\nabla \phi_j\cdot \hat{n} \right\rangle + \left(\psi_i, \frac{\partial \vec{\beta}}{\partial u_j} \cdot\nabla u_h\right) + \left(\psi_i, \vec{\beta} \cdot \nabla \phi_j\right) - \left(\psi_i, \frac{\partial f}{\partial u_j}\right)\end{aligned}$

- Note that even for this "simple" equation, the Jacobian entries are nontrivial: they depend on the partial derivatives of $k$, $\vec{\beta}$, and $f$, which may be difficult or time-consuming to compute analytically.

- In a multiphysics setting with many coupled equations and complicated material properties, the Jacobian might be extremely difficult to determine.

### Chain Rule id=chain_rule

- On the previous slide, the term $\frac{\partial f}{\partial u_j}$ was used, where $f$ was a nonlinear forcing function.

- The chain rule allows us to write this term as

  $\begin{aligned}
    \frac{\partial f}{\partial u_j} &= \frac{\partial f}{\partial u_h} \frac{\partial u_h}{\partial u_j}
    \\
    &=\frac{\partial f}{\partial u_h} \phi_j\end{aligned}$

- If a functional form of $f$ is known, e.g. $f(u) = \sin(u)$, this
  formula implies that its Jacobian contribution is given by

   $\frac{\partial f}{\partial u_j} = \cos(u_h) \phi_j$

### Jacobian-Free Newton-Krylov id=JFNK

- $\mathbf{J}(\vec{u}_n)\delta \vec{u}_{n+1} = -\vec{R}(\vec{u}_n)$ is a linear system solved during each Newton step.
- For simplicity, we can write this linear system as $\mathbf{A}\vec{x} = \vec{b}$, where:
    - $\mathbf{A} \equiv \mathbf{J}(\vec{u}_n)$
    - $\vec{x} \equiv \delta \vec{u}_{n+1}$
    - $\vec{b} \equiv -\vec{R}(\vec{u}_n)$
- We employ an iterative Krylov method (e.g. GMRES) to produce a sequence of iterates $\vec{x}_k \rightarrow \vec{x}$, $k=1,2,\ldots$
- $\mathbf{A}$ and $\vec{b}$ remain *fixed* during the iterative process.
- The "linear residual" at step $k$ is defined as

  $\vec{\rho}_k \equiv \mathbf{A}\vec{x}_k - \vec{b}$

- MOOSE prints the norm of this vector, $\|\vec{\rho}_k\|$, at each iteration, if you set `print_linear_residuals = true` in the `Outputs` block.

- The "nonlinear residual" printed by MOOSE is $\|\vec{R}(\vec{u}_n)\|$.

- By iterate $k$, the Krylov method has constructed the subspace

  $\mathcal{K}_k = \text{span}\{ \vec{b}, \mathbf{A}\vec{b}, \mathbf{A}^2\vec{b}, \ldots, \mathbf{A}^{k-1}\vec{b}\}$

- Different Krylov methods produce the $\vec{x}_k$ iterates in different ways:
- Conjugate Gradients: $\vec{\rho}_k$ orthogonal to $\mathcal{K}_k$.
- GMRES/MINRES: $\vec{\rho}_k$ has minimum norm for $\vec{x}_k$ in $\mathcal{K}_k$.
- Biconjugate Gradients: $\vec{\rho}_k$ is orthogonal to $\mathcal{K}_k(\mathbf{A}^T)$

- $\mathbf{J}$ is never explicitly needed to construct the subspace, only the action of $\mathbf{J}$ on a vector is required.


- This action can be approximated by:
    $\mathbf{J}\vec{v} \approx \frac{\vec{R}(\vec{u} + \epsilon\vec{v}) - \vec{R}(\vec{u})}{\epsilon}$

- This form has many advantages:
    - No need to do analytic derivatives to form $\mathbf{J}$
    - No time needed to compute $\mathbf{J}$ (just residual computations)
    - No space needed to store $\mathbf{J}$


## Solving Linear Systems id=linear_methods

You will commonly hear of two ways to solve an implicit linear system of
equations: directly or iteratively. A typical direct solve will perform a
[LU factorization](https://en.wikipedia.org/wiki/LU_decomposition). Direct
solves are a great tool for solving small-medium sized systems; however, they
are extremely expensive when applied to large-scale problems. To solve
large-scale systems, iterative methods must be used. The most successful
iterative methods are Krylov methods. Krylov methods are
work by finding a solution to $Ax=b$ within a space
called the [Krylov sub-space](https://en.wikipedia.org/wiki/Krylov_subspace)
which is spanned by images of $b$ under powers of $A$. Two of the most used
Krylov algorithms are
[Conjugate gradient](https://en.wikipedia.org/wiki/Conjugate_gradient_method)
and [GMRES](https://en.wikipedia.org/wiki/GMRES). Conjugate gradient generally
only works for symmetric positive-definite matrices. Because of its greater
flexibility, GMRES is the default linear solution algorithm in PETSc and
consequently for MOOSE.

## Augmenting Sparsity id=augmenting_sparsity

One such routine is `NonlinearSystemBase::augmentSparsity`, which as its name
suggests augments the sparsity pattern of the matrix. Currently this method adds
sparsity coming from MOOSE `Constraint` objects. It does this by querying
geometric connectivity information between secondary and primary boundary pairs, and
then querying the `DofMap` attached to the `NonlinearSystemBase` (through the
libMesh `NonlinearImplicitSystem`) for the dof indices that exist on the
elements attached to the secondary/primary nodes. The geometric connectivity
information comes from [`NearestNodeLocators`](/NearestNodeLocator.md) held by
[`GeometricSearchData`](/GeometricSearchData.md) objects in the
[`FEProblemBase`](/FEProblemBase.md) and
[`DisplacedProblem`](/DisplacedProblem.md) (the latter only if there are mesh
displacements). In the future sparsity augmentation from constraints will occur
through [`RelationshipManagers`](/RelationshipManager.md) instead of through the
`augmentSparsity` method.

## Computing Residual and Jacobian Together id=resid_and_jac_together

The default behavior in MOOSE is to have separate functions compute the residual
and Jacobian. However, with the advent of [NonlinearSystem.md#AD] it can make
sense to use a single function to compute the residual and Jacobian
simultaneously. At the local residual object level, automatic differentiation (AD)
already computes the residual and Jacobian simultaneously, with the dual number
at the core of AD holding value (residual) and derivatives
(Jacobian). Simultaneous evaluation of residual and Jacobian using a single
function can be triggered by setting
[!param](/Executioner/Steady/residual_and_jacobian_together) to `true`. What this does in
the background is funnels the (generally AD) computed local residuals and
Jacobians into the global residual vector and Jacobian
matrix respectively when PETSc calls the libMesh/MOOSE residual/function
evaluation routine. Then when PETSc calls the libMesh/MOOSE Jacobian evaluation
routine, we simply return because the global matrix has already been computed.

Computing the residual and Jacobian together has shown 20% gains for
Navier-Stokes finite volume simulations for which automatic differentiation is
leveraged even during standard residual evaluations. Other areas where computing
the residual and Jacobian together may be advantageous is in simulations in
which there are quite a few nonlinear iterations per timestep, for which the
cost of an additional Jacobian evaluation during the final residual evaluation
is amortized. Also, simulations in which material property calculations are very
expensive may be good candidates for computing the residual and Jacobian together.

## Reusing preconditioners id=reuse_preconditioners

The simple version of GMRES and other iterative methods converge only very
slowly.  To improve convergence, PETSc and other iterative solver packages
apply a [preconditioner](https://en.wikipedia.org/wiki/Preconditioner) to the
system of equations/sparse matrix before applying the iterative solver.

A great number of preconditioners exist, but
[multigrid](https://en.wikipedia.org/wiki/Multigrid_method)
methods are often among the best choices for problems without
significant hyperbolic character.
The [HYPRE](application_development/hypre.md optional=true) package,
specifically the
BoomerAMG preconditioner, is often a good choice for a preconditioner to
condition the system of equations resulting from the MOOSE simulation.

A direct factorization of the sparse system of equations is a *very*
good preconditioner.  An iterative method using the factorized matrix
as a preconditioner will typically converge to machine precision in a single
iteration.  However, as noted above, factorizing the sparse system of equations
for a large simulation is numerically expensive.

One option is to form a preconditioner once and then reuse it to solve
the linearized system many times.  The preconditioner can be carried over
through nonlinear iterations and even across time steps.  MOOSE
allows the user to do this with the `reuse_preconditioner` flag.
Setting

```
  reuse_preconditioner = true
  reuse_preconditioner_max_linear_its = 20
```

in the `[Executioner]` block will reuse the same preconditioner until
the number of linear iterations required to solve the linearized system of
equations exceeds 20.   If the number of linear iterations exceeds
`reuse_preconditioner_max_linear_its`
the system does not immediately stop iterating on the current linearized
system.  Instead it will continue until it either successfully solves
the current system or reaches `l_max_its`.  It will then form a new
preconditioner for the next nonlinear iteration.

Using these parameters in combination with a direct factorization of the
system can be very efficient.  The following is an example of how to
direct PETSc and MOOSE to solve the equations with this combination:

```
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -ksp_type'
  petsc_options_value = 'lu superlu_dist gmres'

  reuse_preconditioner = true
  reuse_preconditioner_max_linear_its = 20
```

This solver strategy can be very effective when the system Jacobian
does not change very much from nonlinear iteration to nonlinear iteration
and/or from time step to time step.  The heuristic is also most effective
when the cost of solving the linearized system is a large fraction of
the total simulation time.  As such, it can be especially beneficial
when using an expensive preconditioner, like a direct solver, as shown
in this example.

There are two differences between
`reuse_preconditioner` and
setting up preconditioner reuse directly in PETSc with the
`-snes_lag_preconditioner_persists` and `-snes_lag_preconditioner` options:
1. `-snes_lag_preconditioner X` will recalculate a new preconditioner
   every X linear iterations, regardless of the progress of the linear solve.
   `reuse_preconditioner_max_linear_its = X` will continue to reuse
   the same preconditioner until the number of linear iterations
   required to solve the linearized equations exceeds X.
2. By default libmesh deletes the PETSc `SNES` instance after each time
   step.  This means that regardless of how the reuse options are set,
   the solver cannot retain the preconditioner across time steps.  The
   `reuse_preconditioner` alters this behavior to retain the `SNES`
   instance so that preconditioner reuse can be carried across time
   steps.

Preconditioner reuse is also different from modified Newton methods,
which can be configured with the PETSc `-snes_lag_jacobian` and
`-snes_lag_jacobian_persists` options.  Preconditioner reuse
affects how PETSc solves the linearized system of equations formed
at each nonlinear iteration.  Ideally, if the reused preconditioner
achieves the requested `l_tol` precision before iterating more than
`l_max_its` times, preconditioner reuse will not affect the
convergence of the nonlinear iterations compared to a case with the
reuse option off.  As described above,
preconditioner reuse aims to decrease the time required to solve
the linearized equations at each nonlinear iteration by reducing the
number of times the solver needs to setup the potentially-expensive
linear preconditioner.

By contrast, modified Newton methods will affect the nonlinear
convergence of the system without affecting how PETSc solves the
linearized system of equations.  The goal of
modified Newton methods is to reduce the time required to solve
the nonlinear equations by forming a new Jacobian matrix less often.

Put another way, preconditioner reuse aims to speed up solving the
linear system of equations while modified Newton methods aim to
accelerate solving the nonlinear equations.

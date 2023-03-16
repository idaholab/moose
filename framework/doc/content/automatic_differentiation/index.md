# Automatic Differentiation

## Background

Historically, the most common question on the MOOSE mailing list has been
"why is my solve not converging?" The equivalent question is also posted on
the Computational Science StackExchange under the title
[Why is Newton's method not converging?](https://scicomp.stackexchange.com/questions/30/why-is-newtons-method-not-converging)
The leading bullet in the accepted answer is
that the Jacobian is wrong. Coding Jacobians can be a difficult and tedious
task, especially for physics that require complex material
models. Instead of spending their time running simulations and generating results,
physicists and engineers may sink days or weeks into constructing accurate
Jacobians. Oftentimes, the developer will just throw up their hands and elect an
approximate Jacobian method like [!ac](PJFNK) where the
Jacobian is never explicitly formed but instead its action on vectors is
approximated using finite differences and an approximate
preconditioning matrix is hand coded for
constructing an effective preconditioner.
While effective in many cases, the
quality of the matrix-free approximation is closely tied to selection of
a differencing parameter whose size should vary based on the noisiness of the
nonlinear system of equations. If the nonlinear functions are noisy and too
small of a differencing parameter is chosen, truncation will lead to a Jacobian
approximation that is actually a non-linear operator. If the differencing
parameter is too large, then the approximated derivatives will be inaccurate if
the differenced function is nonlinear. For multi-physics problems in which the
magnitudes of solution and residual components may vary wildly, an arbitrary
choice of differencing parameter may lead to an accurate approximation of the
Jacobian action for one physics but lead to the aforementioned truncation error
in another. The presence of truncation error and hence a non-linear operator is
evident in a linear solve derived from porous flow equations
coupled with heat transport:

```text
 0 KSP unpreconditioned resid norm 5.645573426379e+16 true resid norm 5.645573426379e+16 ||r(i)||/||b|| 1.000000000000e+00
 1 KSP unpreconditioned resid norm 2.965618482980e+12 true resid norm 2.965618483296e+12 ||r(i)||/||b|| 5.252997807874e-05
 2 KSP unpreconditioned resid norm 2.724648158452e+12 true resid norm 2.890172312561e+14 ||r(i)||/||b|| 5.119360061914e-03
 3 KSP unpreconditioned resid norm 8.335608026448e+11 true resid norm 5.178511494633e+15 ||r(i)||/||b|| 9.172693548605e-02
 4 KSP unpreconditioned resid norm 8.335450917197e+11 true resid norm 8.675908349302e+17 ||r(i)||/||b|| 1.536762998912e+01
 5 KSP unpreconditioned resid norm 6.022626374152e+11 true resid norm 1.028328406646e+22 ||r(i)||/||b|| 1.821477339823e+05
 6 KSP unpreconditioned resid norm 6.020981842850e+11 true resid norm 1.028926824260e+22 ||r(i)||/||b|| 1.822537316498e+05
 7 KSP unpreconditioned resid norm 2.079735227207e+11 true resid norm 6.050540886054e+21 ||r(i)||/||b|| 1.071731855932e+05
```

While the unpreconditioned residual norm, produced through [!ac](GMRES) iterations,
drops by five orders of magnitude during the solve, the true residual norm
computed via $\mathbf{A}\vec{x} - \vec{b}$ actually increases by five orders of
magnitude. In MOOSE, a right preconditioned [!ac](GMRES)
is chosen by default, where the unpreconditioned residual mathematically equal to
the actual residual if the preconditioning matrix is well-conditioned. The inconsistencies
generated here are, partially, because of the inaccuracy of the preconditioning matrix.
The net result of such bad linear solves is a diverging Newton's
method since the computed Newton update is "wrong":

```text
0 Nonlinear |R| = 1.138921e+06
1 Nonlinear |R| = 1.392349e+11
2 Nonlinear |R| = 2.881060e+10
3 Nonlinear |R| = 8.473409e+09
4 Nonlinear |R| = 2.017729e+09
5 Nonlinear |R| = 7.634503e+08
6 Nonlinear |R| = 5.645573e+16
Nonlinear solve did not converge due to DIVERGED_FNORM_NAN iterations 6
```

If we instead form an explicit Jacobian and eschew finite differencing, Newton's
method converges, albeit sub-quadratically since the hand-coded Jacobian is inaccurate:

```text
 0 Nonlinear |R| = 1.138921e+06
 1 Nonlinear |R| = 2.171654e+05
 2 Nonlinear |R| = 4.550729e+04
 3 Nonlinear |R| = 1.026123e+04
 4 Nonlinear |R| = 1.267614e+03
 5 Nonlinear |R| = 6.531982e+01
 6 Nonlinear |R| = 4.427446e-01
 7 Nonlinear |R| = 8.025639e-05
 8 Nonlinear |R| = 3.195008e-06
 9 Nonlinear |R| = 1.669770e-07
10 Nonlinear |R| = 1.201906e-08
Nonlinear solve converged due to CONVERGED_FNORM_ABS iterations 10
```

Given the multi-physics aim of MOOSE and the clear pitfalls associated with
differencing approximations, there is clear motivation to form accurate explicit
representations of the matrix. Note, even if a perfect Jacobian action can be
achieved via the finite difference scheme, a suitable preconditioning matrix
is required to construct a robust and efficient solver.

Some users elect to use symbolic differentiation
packages like SymPy or
Mathematica; however, for functions of any
complexity the resulting gradient expressions can take up to several pages and
can be quite difficult to translate from notebook to
code. An alternative to numerical and symbolic
differentiation is [!ac](AD), which applies the chain rule to
elementary operations at every step of the computer program and which applies at
most a small constant factor (estimated to
have an upper bound of 5) of additional
arithmetic operations. Weighed against the untold number of developer hours
sunk into attempting to create accurate hand-coded Jacobians and analyst hours
spent waiting for problems with poor hand-coded or approximated Jacobians to
converge, the small additional CPU cost imposed by [!ac](AD) seemed well worth
the trade. With an accurate Jacobian formed using [!ac](AD), the overall simulation
 can be much faster than that utilizing a deficient hand-coded matrix.
Having elected to pursue AD, there was a choice between forward
and reverse modes. Forward mode [!ac](AD) is best suited for problems with many more
outputs than inputs; reverse mode is best suited for many more inputs than
outputs. The latter case is more prevalent in deep learning applications and is
what is implemented in popular machine learning libraries like
PyTorch. In solution of nonlinear systems of
equations the number of inputs and outputs are equivalent, so the choice is not
clear-cut. However, given the architecture of MOOSE in which the residuals
are constructed from finite element solutions, which themselves are naturally constructed
from the nonlinear degrees of freedom, forward propagation was the logical
choice. Forward-mode AD relies on the concept of `DualNumbers`
which can be implemented either through source code transformation or operator
overloading. The latter is more easily implemented in programming languages that
support it such as C++, the language that MOOSE is written
in. Conveniently, the C++ header-only library MetaPhysicL came
ready-made with a `DualNumber`  template class and an operator-overload [!ac](AD)
implementation that fit into the MOOSE architecture with minimal
disruption to the code-base. The [!ac](AD) capability of MetaPhysicL was
merged into the MOOSE code-base in the fall of 2018.

## AD implementation - MetaPhysicL

The automatic differentiation classes in MetaPhysicL were
originally developed and tested in the Manufactured Analytical
Solution Abstraction (`MASA`) library,
used for generating manufactured solution tests for increasingly
complicated physics simulations, when it was discovered that multiple
symbolic differentiation packages were suffering software failures on
sufficiently large problems.  Symbolically differentiating
manufactured solution fields through e.g.\ 3-D Navier-Stokes physics
caused a combinatorial explosion, leading to corresponding forcing
functions that were hundreds of kilobytes in length, or required many
man-hours of manual simplification, or failed altogether on some
Computer Algebra System software.  Automatic differentiation allowed
for the generation of manufactured solution forcing functions using
code that was hardly more complex than the physics equations
themselves. The classes used for this effort were eventually published as an
independent library, MetaPhysicL, for wider use and further
development.

`DualNumber` is the centerpiece class of the automatic differentiation
capability. It stores `value` and `derivatives` members which
correspond to $f(\vec{x})$ and $\nabla f(\vec{x})$ respectively. `value`
and `derivatives` types are determined by `T` and `D`
template parameters, where `T` is some floating point type, and
`D` is equivalent to `T` for single-argument functions or equal to
some container type for a generic vector of arguments. MetaPhysicL
overloads binary arithmetic operators (`+,-,*,/`), unary functions
(`std::sin, std::cos, std::exp` etc.), and binary functions
(`std::pow, std::max, std::min` etc.), ensuring that any calculation
involving a `DualNumber` propagates both the function value and its
derivatives.

MOOSE leverages one of two MetaPhysicL container class templates depending
on user configuration. The default MOOSE configuration uses the
`NumberArray` class template which accepts `std::size_t N` and
`typename T` template arguments where `N` denotes the length of
an underlying C-array that holds the `NumberArray` data and `T` is
the floating-point type held by the C-array. As for `DualNumber`,
MetaPhysicL provides arithmetic, unary, and binary function overloads for
manipulation of its container types. `NumberArray` is an ideal derivative
container choice when there is dense coupling between physics variables; this is because
operator and function overloads for `NumberArray` operate on the entire
underlying C-array. The second MetaPhysicL container class leveraged by
MOOSE is `SemiDynamicSparseNumberArray`, which is a more ideal
choice for problems in which variable coupling is sparse or when a user wishes
to solve a variety of problems with a single library configuration. In contrast to
`NumberArray` which only holds a single C-array of floating-type data,
`SemiDynamicSparseNumberArray` additionally holds an array (as a `std::array`)
of integers corresponding to dof indices. The existence of this
additional data member enables sparse operations that may involve only a subset
of the elements in the underlying floating-point data. As an explicit example of
when these sparse operations are useful, consider a
user who may configure MOOSE with an underlying derivative storage container
size of 81 for solid mechanics simulations on 3D second-order hexagonal finite elements
(3 displacement variables * 27 degrees of freedom per variable per finite
element = 81 local dofs). When running 3D, second-order cases,
the non-sparse `NumberArray` container would be 100\% efficient. However,
if the user wishes to run a 2D, second-order case with the same MOOSE
configuration, they would be performing $81 / 18 = 4.5$ times more work
than is necessary if using `NumberArray`. Because
`SemiDynamicSparseNumberArray` tracks the sparsity pattern, it will only
initialize and operate on the floating-point array elements that are required,
e.g. the ``sparse size'' (stored as a `_dynamic_N` data member) of its
data containers will never exceed what is required for the run-time problem,
e.g. 18 for the 2D second-order solid mechanics example. Of course tracking the
sparsity pattern has non-zero cost, so if the user knows they will always be
running a certain kind of problem, they may be best served by configuring with
non-sparse `NumberArray` container.

## AD in MOOSE

As mentioned in above, MetaPhysicL is a forward-mode [!ac](AD)
package. For a finite element framework like MOOSE derivative seeding
begins when constructing local finite element
solutions. The finite element solution approximation is given by
$u_h = \sum_i^Nu_i\phi_i$ where the $u_i's$ represent the degrees of freedom and
the $\phi_i's$ are shape functions associated with the dofs. For a
Lagrange basis, shape functions and dofs are tied to mesh nodes. To
illustrate initiation of the AD process, we will consider construction of
a local finite element solution on a `QUAD4` element, that is to say a
quadrilaterial with a number of nodes equal to the number of vertices. This
element type when combined with a Lagrange basis has four dofs which
contribute to the local solution, one for each element node. In MOOSE we
assign these local degree of freedom solution values (the local $u_i's$) to a
variable class data member called `_ad_dof_values` where the `ad`
prefix denotes automatic differentiation. We then seed a derivative
value of $1$ (recognizing that $\frac{\partial u_i}{\partial u_j} = 1$ when
$i = j$)  at a corresponding local dof index determined through a
somewhat arbitrary numbering scheme. We choose a variable major numbering scheme
such that the local degrees of freedom are in a contiguous block for each
variable, e.g. if we have two variables in the system, $u$ and $v$, then the
numbering scheme for a `QUAD4` element with Lagrange basis would look like
$u_0,u_1,u_2,u_3,v_0,v_1,v_2,v_3$ with subscripts corresponding to the local
node number. We can examine the dependence of the local finite element solution
on each degree of freedom for an arbitrary point in the domain; we know
analytically the expected derivatives:
$\frac{\partial u_h}{\partial u_j} = \phi_j$. For a chosen Gaussian integration
point $(\xi,\eta) = (-.577, -.577)$, we know the corresponding Lagrange $\phi$ values:
$\phi_0=.622,\phi_1=.167,\phi_2=.0447,\phi_3=.167$, and we can check and verify
whether our automatically differentiated solution `_ad_u` matches:

```text
(lldb) p _ad_u[0]
(MetaPhysicL::DualNumber<double, MetaPhysicL::SemiDynamicSparseNumberArray<double, unsigned int, MetaPhysicL::NWrapper<50> >, false>) $10 = {
  _val = 0
  _deriv = {
    MetaPhysicL::DynamicSparseNumberBase<MetaPhysicL::DynamicStdArrayWrapper<double, MetaPhysicL::NWrapper<50> >, MetaPhysicL::DynamicStdArrayWrapper<unsigned int, MetaPhysicL::NWrapper<50> >, MetaPhysicL::SemiDynamicSparseNumberArray, double, unsigned int, MetaPhysicL::NWrapper<50> > = {
      _data = {
        _data = {
          _M_elems = {
            [0] = 0.62200846792814624
            [1] = 0.16666666666666669
            [2] = 0.044658198738520456
            [3] = 0.16666666666666669
            [4] = 0
            [5] = 4.82252338622924E-317
            [6] = 4.8224680508769058E-317
            [7] = 3.9525251667299724E-323
```

Note that some of the unused values in indices 4--7 appear to contain
garbage. This is actually desirable; it indicates that for the
`SemiDynamicSparseNumberArray` container, unnecessary components of the
derivative vector are left uninitialized.

We can also check variable coupling. Let us assume a reaction-type problem in
which the variable $u$ disappears at a rate directly proportional to its
concentration and appears at at rate directly proportional to the concentration
of the variable $v$. The strong form of this residual is then simply
$u - v$. Examining the derivatives of this term produced by automatic
differentiation

```text
(lldb) p strong_residual
(MetaPhysicL::DualNumber<double, MetaPhysicL::SemiDynamicSparseNumberArray<double, unsigned int, MetaPhysicL::NWrapper<50> >, false>) $1 = {
  _val = 0
  _deriv = {
    MetaPhysicL::DynamicSparseNumberBase<MetaPhysicL::DynamicStdArrayWrapper<double, MetaPhysicL::NWrapper<50> >, MetaPhysicL::DynamicStdArrayWrapper<unsigned int, MetaPhysicL::NWrapper<50> >, MetaPhysicL::SemiDynamicSparseNumberArray, double, unsigned int, MetaPhysicL::NWrapper<50> > = {
      _data = {
        _data = {
          _M_elems = {
            [0] = 0.62200846792814624
            [1] = 0.16666666666666669
            [2] = 0.044658198738520456
            [3] = 0.16666666666666669
            [4] = -0.62200846792814624
            [5] = -0.16666666666666669
            [6] = -0.044658198738520456
            [7] = -0.16666666666666669
```

we see exactly what we would expect: values 0--3 corresponding to the $u$
indices are equivalent to that shown in the previous lldb output whereas the
values in 4--7, corresponding to the $v$ indices, are equal to the negative of
that shown in the previous lldb output. In general, the quality of automatic
differention results are verified with unit testing in MetaPhysicL and using a
`PetscJacobianTester` in MOOSE which compares the Jacobian produced
by automatic differentiation against that generated using finite differencing of
the residuals. The latter test relies on using well-scaled problems; for
poorly-scaled problems floating point errors can result in a loss in accuracy of
the finite-differenced Jacobian.

### Using AD in MOOSE

Leveraging the automatic differentiation capabilities in MOOSE is as simple as
inheriting from our AD base classes, e.g. `ADKernel, ADIntegratedBC, ADDGKernel,
ADNodalBC`, etc. The only method an application developer has to override is
`computeQpResidual` or if deriving from `ADKernelGrad` or `ADKernelValue`, then
override `precomputeQpResidual`. To couple in AD variables, the application
developer should use methods like `adCoupledValue, adCoupledGradient,
adCoupledSecond` etc. inherited through the [`Coupleable`](/Coupleable.md)
interface. Material properties with automatic differentiation info can be
created in `Material` classes by using the `declareADProperty` API. [!ac](AD) material
properties can be retrieved in compute objects like `ADKernels` by using the
`getADMaterialProperty` API. For detailed examples of [!ac](AD) use, the reader is
encouraged to investigate the tensor mechanics, navier-stokes, and level-set
modules, all of which heavily leverage MOOSE's [!ac](AD) capabilities.

### Combining AD and non-AD classes

It is possible to support the use of AD and non-AD variables in classes without
having to duplicate code unnecessarily through the use of templating. Several
examples can be found in the code base, the details of which are outlined
[here](templated_objects.md).

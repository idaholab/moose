# FEProblemBase

The FEProblemBase class is an intermediate base class containing all of the common
logic for running the various MOOSE simulations. MOOSE has two built-in types of
problems [FEProblem.md] for solving "normal" physics problems and [EigenProblem.md]
for solving Eigenvalue problems. Additionally, MOOSE contains an [ExternalProblem.md]
problem useful for creating ["MOOSE-wrapped Apps"](moose_wrapped_apps.md optional=True).

!syntax description /Problem/FEProblem

## Convenience Zeros

One of the advantages of the MOOSE framework is the ease at building up Multiphysics
simulations. Coupling is a first-class feature and filling out residuals, or
material properties with coupling is very natural. When coupling is optional, it
is often handy to have access to valid data structures that may be used in-place
of the actual coupled variables. This makes it possible to avoid branch statements
inside of your residual statements and other computationally intensive areas of
code. One of the ways MOOSE makes this possible is by making several different
types of "zero" variables available. The following statements illustrate how
optional coupling may be implemented with these zeros.

```cpp
// In the constructor initialization list of a Kernel

  _velocity_vector(isParamValid("velocity_vector") ? coupledGradient("velocity_vector") : _grad_zero)


// The residual statement

  return _test[_i][_qp] * (_velocity_vector[_qp] * _grad_u[_qp]);
```

## Selective Reinit

The system automatically determines which variables should be made available for use on the
current element ("reinit"-ed). Each variable is tracked on calls through the coupling interface.
Variables that are not needed are simply not prepared. This can save significant amounts
of time on systems that have several active variables.

## Finite Element Concepts

### Shape Functions id=shape_functions

- While the weak form is essentially what you need for adding physics to MOOSE, in traditional finite element software more work is necessary.
- We need to discretize our weak form and select a set of simple "basis functions" amenable for manipulation by a computer.

!media media/problems/basis-function-example.jpg
       caption=Example of linear Lagrange shape function associated with single
       node on triangular mesh
       style=width:50%;

!media media/problems/Oden-linear-lagrange.jpg
       caption=1D linear Lagrange shape functions
       style=width:25%;

- Our discretized expansion of $$u$$ takes on the following form:
  $u \approx u_h = \sum_{j=1}^N u_j \phi_j$
- The $\phi_j$ here are called "basis functions"
- These $\phi_j$ form the basis for the "trial function", $u_h$
- Analogous to the $x^n$ we used earlier
- The gradient of $u$ can be expanded similarly:
   $\nabla u \approx \nabla u_h = \sum_{j=1}^N u_j \nabla \phi_j$
- In the Galerkin finite element method, the same basis functions are used for both the trial and test functions:
    $\psi = \{\phi_i\}_{i=1}^N$
- Substituting these expansions back into our weak form, we get:
    $\left(\nabla\psi_i, k\nabla u_h \right) - \langle\psi_i, k\nabla u_h\cdot \hat{n} \rangle +
       \left(\psi_i, \vec{\beta} \cdot \nabla u_h\right) - \left(\psi_i, f\right) = 0, \quad i=1,\ldots,N$
- The left-hand side of the equation above is what we generally refer to as the $i^{th}$ component of our "Residual Vector" and write as $R_i(u_h)$.
- Shape Functions are the functions that get multiplied by coefficients and summed to form the solution.
- Individual shape functions are restrictions of the global basis functions to individual elements.
- They are analogous to the $x^n$ functions from polynomial fitting (in fact, you can use those as shape functions).
- Typical shape function families: Lagrange, Hermite, Hierarchic, Monomial, Clough-Toucher
    - MOOSE has support for all of these.
- Lagrange shape functions are the most common.
    -  They are interpolary at the nodes, i.e., the coefficients correspond to the values of the functions at the nodes.

#### Example 1D Shape Functions

!media media/problems/linear-lagrange-graph.jpg
       style=width:50%;

Linear Lagrange

!media media/problems/quadratic-lagrange-graph.jpg
       style=width:50%;

Quadratic Lagrange

!media media/problems/cubic-lagrange-graph.jpg
       style=width:50%;

Cubic Lagrange

!media media/problems/cubic-hermite-graph.jpg
       style=width:50%;

Cubic Hermite

#### 2D Lagrange Shape Functions

Example bi-quadratic basis functions defined on the Quad9 element:

- $\psi_0$ is associated to a "corner" node, it is zero on the opposite edges.
- $\psi_4$ is associated to a "mid-edge" node, it is zero on all other edges.
- $\psi_8$ is associated to the "center" node, it is symmetric and $\geq 0$ on the element.

!media media/problems/quad9-corner.jpg
       style=width:50%;

$\psi_0$

!media media/problems/quad9-edge.png
       style=width:50%;

$\psi_4$

!media media/problems/quad9-center.jpg
       style=width:50%;

$\psi_8$

### Numerical Integration id=numerical_integration

- The only remaining non-discretized parts of the weak form are the integrals.
- We split the domain integral into a sum of integrals over elements:
  $\int_{\Omega} f(\vec{x}) \;\text{d}\vec{x} = \sum_e \int_{\Omega_e} f(\vec{x}) \;\text{d}\vec{x}$
- Through a change of variables, the element integrals are mapped to integrals over the "reference" elements $\hat{\Omega}_e$.
  $\sum_e \int_{\Omega_e} f(\vec{x}) \;\text{d}\vec{x} =
        \sum_e \int_{\hat{\Omega}_e} f(\vec{\xi}) \left|\mathcal{J}_e\right| \;\text{d}\vec{\xi}$
- $\mathcal{J}_e$ is the Jacobian of the map from the physical element to the reference element.
- To approximate the reference element integrals numerically, we use quadrature (typically "Gaussian Quadrature"):
  $\sum_e \int_{\hat{\Omega}_e} f(\vec{\xi}) \left|\mathcal{J}_e\right| \;\text{d}\vec{\xi} \approx
        \sum_e \sum_{q} w_{q} f( \vec{x}_{q}) \left|\mathcal{J}_e(\vec{x}_{q})\right|$
- $\vec{x}_{q}$ is the spatial location of the $q$th quadrature point and $w_{q}$ is its associated weight.
- MOOSE handles multiplication by the Jacobian and the weight automatically, thus your `Kernel` is only responsible for computing the $f(\vec{x}_{q})$ part of the integrand.
- Under certain common situations, the quadrature approximation is exact!
    - For example, in 1 dimension, Gaussian Quadrature can exactly integrate polynomials of order $2n-1$ with $n$ quadrature points.
- Note that sampling $u_h$ at the quadrature points yields:
   $\begin{aligned}
    u(\vec{x}_{q}) &\approx u_h(\vec{x}_{q}) = \sum u_j \phi_j(\vec{x}_{q}) \\
    \nabla u (\vec{x}_{q}) &\approx \nabla u_h(\vec{x}_{q}) = \sum u_j \nabla \phi_j(\vec{x}_{q})\end{aligned}$
- And our weak form becomes:
  $\begin{aligned}
  R_i(u_h) &= \sum_e \sum_{q} w_{q} \left|\mathcal{J}_e\right|\left[ \nabla\psi_i\cdot k \nabla u_h + \psi_i \left(\vec\beta\cdot \nabla u_h \right) - \psi_i f \right](\vec{x}_{q}) \\
  &- \sum_f \sum_{q_{face}} w_{q_{face}} \left|\mathcal{J}_f\right| \left[\psi_i k \nabla u_h \cdot \vec{n} \right](\vec x_{q_{face}})
  \end{aligned}$
- The second sum is over boundary faces, $f$.
- MOOSE `Kernels` must provide each of the terms in square brackets (evaluated at $\vec{x}_{q}$ or $\vec x_{q_{face}}$ as necessary).


!syntax parameters /Problem/FEProblem

!alert note
When [!param](/Problem/FEProblem/check_uo_aux_state) is set to true, MOOSE will evaluate user objects
(including all postprocessors and vector postprocessors) and auxiliary kernels on every execute flag except `linear`
twice. It then compares the results after two evaluations including the auxiliary system solutions and all the real reporter values added by user objects.
When MOOSE sees a difference, it will issue an error indicating that there are unresolved dependencies
during the evaluation because otherwise the results should only depend on primary solutions and should not change.
MOOSE also prints the details about the difference to help users removing the states caused by these unresolved dependencies.

!syntax inputs /Problem/FEProblem

!syntax children /Problem/FEProblem

!bibtex bibliography

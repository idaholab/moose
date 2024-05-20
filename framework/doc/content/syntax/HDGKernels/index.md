# HDG Kernels

HDG kernels and their boundary condition counterparts,
[HDGBCs/index.md], are advanced systems that should only be developed by
users with a fair amount of finite element experience. For background on
hybridization, we encourage the user to read [!citep](cockburn2009unified) which
presents a unified framework for considering hybridization of discontinuous
Galerkin, mixed, and continuous Galerkin methods for elliptic
problems. [!citep](cockburn2008superconvergent) presents a single-face
hybridizable discontinuous Galerkin (HDG) method for an elliptic problem, in which a
non-zero stabilization term is added to only one face of a given
element. [!citep](nguyen2010hybridizable) presents an HDG method for Stokes
flow. [!citep](nguyen2011implicit) extends HDG to Navier-Stokes. More HDG
literature may be found by looking at the research of Bernardo Cockburn, his
former postdoc Sander Rhebergen, and Rhebergen's former postdoc Tamas
Horvath. Work by Tan Bui-Thanh on upwind HDG methods, like in
[!citep](bui2015godunov) is also worth noting.

A hybridized finite element formulation starts with some primal finite element
discretization. Then some continuity property of the finite element space is
broken. For instance Raviart-Thomas finite elements may be used to solve a mixed
formulation description of a Poisson problem. The Raviart-Thomas elements ensure
continuity of the normal component of the vector field across element faces. We
break that continuity in the finite element space used in the hybridized method
and instead introduce degrees of freedom, that live only on the mesh skeleton
(the faces of the mesh), that are responsible for ensuring the continuity that
was lost by breaking the finite element space. In libMesh/MOOSE implementation
terms, when hybridizing the Raviart-Thomas description of the Poisson problem,
we change from using a `RAVIART_THOMAS` basis to an `L2_RAVIART_THOMAS` basis
and introduce a `SIDE_HIERARCHIC` variable whose degrees of freedom live on the
mesh skeleton. We will refer to the variables that exist "before" the
hybridization as primal variables and the variable(s) that live on the mesh
skeleton as Lagrange multipliers (LMs) or dual variable(s).

We note that some classes of HDG methods, such as the LDG method in
[!citep](cockburn2008superconvergent), have the gradient as an independent
primal variable. With these methods, for diffusion or diffusion-dominated
problems, the primal gradient and primal scalar variable fields can be used to
postprocess a scalar field that converges with order $k + 2$ in the $L^2$ norm,
where $k$ is the polynomial order of the primal scalar variable. However, as
advection becomes dominant, the higher order convergence is lost and
consequently so is the value of having the gradient as an independent
variable. In advection-dominated cases, interior penalty HDG methods, such as
that outlined in [!citep](rhebergen2017analysis), may be a good choice.

## Implementation in MOOSE

HDG kernels derive from [Kernels](Kernels/index.md). However, the methods
that must be overridden are quite different. These are `onElement` and
`onInternalSide`, which implement integrations in the volume of elements and on
internal faces respectively. External boundary condition integration occurs in
[HDGBCs/index.md].

Within `onElement` and `onInternalSide`, hybridized kernel developers have eight
different data structures they need to populate. Six are inherited from the `HDGData`
class). These are

```
  /// Matrix data structures for on-diagonal coupling
  EigenMatrix _PrimalMat, _LMMat;
  /// Vector data structures
  EigenVector _PrimalVec, _LMVec;
  /// Matrix data structures for off-diagonal coupling
  EigenMatrix _PrimalLM, _LMPrimal;
```
And the two declared in `HDGKernel`:
```
  /// Containers for the global degree of freedom numbers for primal and LM variables
  /// respectively
  std::vector<dof_id_type> _primal_dof_indices;
  std::vector<dof_id_type> _lm_dof_indices;
```

The `_PrimalMat` holds the Jacobian entries for the dependence of primal degrees
of freedom on primal degrees of freedom; `_LMMat` is dependence of LM dofs on LM
dofs; `_PrimalLM` is dependence of primal dofs on LM dofs; `_LMPrimal` is
dependence of LM dofs on primal dofs. The `_PrimalVec` and `_LMVec` objects hold
the residuals for the primal and LM degrees of freedom
respectively. `_primal_dof_indices` and `_lm_dof_indices` hold the primal and LM
global degree of freedom numbers respectively for the current
element. `HDGIntegratedBC` classes also inherit from `HDGData` and must also fill
the six matrix and vector structures within their `onBoundary` method.

Note that local finite element assembly occurs twice within a single iteration
of Newton's method. The first assembly occurs prior to the linear solve and adds
into the global residual and Jacobian data structures which represent only the
trace/Lagrange-multiplier degrees of freedom. The linear solve then occurs which
computes the Newton update for the Lagrange multiplier degrees of freedom. This
Lagrange multiplier increment is then used in the second assembly
post-linear-solve to compute the primal variable solution increment. Because
only the Lagrange multiplier variables and their degrees of freedom participate
in the global solve, they are the only variables that live in the nonlinear
system. The primal variables live in the auxiliary system.

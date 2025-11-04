# HDG Kernels

HDG kernels are an advanced systems that should only be developed by
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

HDG kernels derive from [Kernels](Kernels/index.md). However, they add additional interfaces:
`computeResidualOnSide` and `computeJacobianOnSide` which must be overridden and
`computeResidualAndJacobianOnSide` which may be optionally overridden if the HDG kernel developer
wishes to enable the ability to compute the residual and Jacobian together. These interfaces will be
called on internal faces on a per-element basis. This means that a given internal face will be
visited twice, once from each element side. External boundary condition integration occurs with
standard boundary condition classes, see [syntax/BCs/index.md].

There are currently two HDG implementations in MOOSE: L-HDG and IP-HDG. Both L-HDG and IP-HDG kernel
classes inherit from `HDGKernel` but that is where their similarity ends. L-HDG currently implements
physics monolithically, e.g. the L-HDG discretization of the Navier-Stokes equations, both mass and
momentum, is contained entirely within a single kernel. However, the MOOSE IP-HDG implementation is
modular; like is typically the case in MOOSE, there is a single kernel object per PDE term. So for a
2D setup of the Navier-Stokes equations, there are five kernels. Two advection kernels for the x-
and y-momentum component equations, two stress kernel (which contains both viscous stress and
pressure) for the x- and y-momentum component equations, and one advection kernel for the mass
equation.

This difference in kernel design naturally has consequences for boundary conditions. The monolithic
kernel design for L-HDG leads to monolithic boundary conditions. Modularity for IP-HDG kernels means
modular boundary conditions, e.g. a user may end up specifying multiple integrated boundary
conditions on a single boundary like is done in

!listing hdgkernels/ip-advection-diffusion/mms-advection-diffusion.i block=BCs

In the future we may make L-HDG design more modular, although the monolithic approach does have the
advantage of less required user input. However, less user input may also be achieved in the future
by leveraging the [Physics system][/Physics/index.md].

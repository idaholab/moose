# Stabilization for the Lagrangian Kernels

## The Need for Stabilization

Standard 2D and 3D finite elements using a linear interpolation of
the displacement field are not stable for incompressible
and nearly incompressible deformation [!cite](hughes1987finite).
These standard elements include the triangle, tetrahedral, quadrilateral,
and hexahedral elements commonly used in solving problems in the 
Tensor Mechanics module.  Incompressible deformation is 
material deformation that does not change the local volume of the structure,
for example linear elastic deformation as the Poisson's ratio
approaches $\nu = 0.5$.  Other common examples include problems
representing elastic-plastic materials
problems with widespread plasticity, as (traditional) plasticity occurs
via shear, as well as many types of hyperelastic models representing soft polymers.

Under these conditions standard linear elements exhibit volumetric locking
where the apparent, numerical stiffness of the element is much greater
than the actual analytic stiffness of the structure.  This locking
leads to inaccurate results which do not improve with mesh refinement.

## $\bar{F}$ and $\bar{B}$ Stabilization

There are two common methods used to stabilize linear elements and 
avoid volumetric locking: the $\bar{F}$ [!cite](de1996design)
and $\bar{B}$ [!cite](hughes1980generalization).  Of these theories
$\bar{B}$ is older and was originally developed for small deformation
problems while $\bar{F}$ was developed later and originally intended for
large deformation problems.  However, the $\bar{F}$ method can be used
for small deformation problems as well, $\bar{B}$ can be extended to
large deformation problems, and the $\bar{F}$ method 
can be viewed as a subset of the $\bar{B}$ theory.  The following
explains the two approaches in the context of small deformations.

$\bar{F}$ alters the definition of the strain being fed into the
constitutive models to produce the stress, subsequently used by the kernel
to calculate the stress equilibrium residual.  The theory modifies
the strains so that the dilitational part of the
strain at each quadrature point is set equal to the volume-average 
dilitation strain.  Mathematically,
\begin{equation}
      \bar{\varepsilon}_{ij}=\frac{1}{v}\int_{v}\varepsilon_{ij}dv
\end{equation}
\begin{equation}
      \varepsilon_{ij}^\prime=\varepsilon_{ij}+\frac{1}{3}\left(\bar{\varepsilon}_{kk}-\varepsilon_{kk}\right)\delta_{ij}
\end{equation}
where $\varepsilon_{ij}$ is the strain calculated from the displacement gradient.
This method then stabilizes the problem by replacing the linear-varying dilitational strain with 
a constant dilitational strain over each element.
Notionally, in MOOSE the $\bar{F}$ only alters the material model, though in fact it also changes the
definition of the Jacobian (but not the residual) in the [Kernel](Kernel.md).

$\bar{B}$ makes this modification to the strain but then also modifies the definition of
the residual, replacing the [original](LagrangianKernelTheory.md) small deformation stress equilibrium weak form
\begin{equation}
      R^{\alpha}=\int_{v}s_{ij}\phi_{i,j}^{\alpha}dv
\end{equation}
with a modified version
\begin{equation}
      R^{\alpha}=\int_{v}s_{ij}\phi_{i,j}^{\prime \alpha}dv
\end{equation}
where the method modifies the trial function gradient $\phi_{i,j}^{\alpha}$
in exactly the same way as the strains:
\begin{equation}
      \bar{\phi}_{i,j}^{\alpha}=\frac{1}{v}\int_{v}\phi_{i,j}^{\alpha} dv
\end{equation}
\begin{equation}
      \phi_{i,j}^{\prime \alpha}=\phi_{i,j}+\frac{1}{3}\left(\bar{\phi}_{k,k}^{\alpha}-\phi_{k,k}^{\prime \alpha}\right)\delta_{ij}.
\end{equation}

The $\bar{B}$ modification results in a symmetric Jacobian (assuming that the original problem had a symmetric Jacobian).
This is a significant advantage for codes taking advantage of the symmetry of the assembled Jacobian matrix.
However, MOOSE does not take advantage of this symmetry and so the Lagrangian kernel system implements the $\bar{F}$ method, as
it is somewhat easier to derive and implement in the large deformation context.

## Implementation in the Lagrangian Kernel System

The `stabilize_strain` flag controls stabilization in the Lagrangian kernel system.  This flag must be set for
both the stress equilibrium kernels [TotalLagrangianStressDivergence](kernels/lagrangian/TotalLagrangianStressDivergence.md) or
[UpdatedLagrangianStressDivergence](/UpdatedLagrangianStressDivergence.md) and the strain calculator 
[`ComputeLagrangianStrain`](ComputeLagrangianStrain.md).
The values must be consistent.

The form of the stabilization depends on if the problem is using large or small displacement kinematic theory,
controlled in turn by the value of the `large_kinematics` flag passed to both the kernel and strain calculator.

For small displacements the strain calculator modifies the strains in the manner described in the previous subsection:
\begin{equation}
      \bar{\varepsilon}_{ij}=\frac{1}{v}\int_{v}\varepsilon_{ij}dv
\end{equation}
\begin{equation}
      \varepsilon_{ij}^\prime=\varepsilon_{ij}+\frac{1}{3}\left(\bar{\varepsilon}_{kk}-\varepsilon_{kk}\right)\delta_{ij}
\end{equation}

For large displacements the strain calculator modifies the deformation gradient instead:
\begin{equation}
      \bar{F}_{iJ}=\frac{1}{V}\int_{V}F_{iJ}dV
\end{equation}
\begin{equation}
      F_{iJ}^{\prime}=\left(\frac{\det\bar{F}}{\det F}\right)^{1/3}F_{iJ}.
\end{equation}

From here the stress update proceeds the same as with stabilization off, except the stress is now based on the modified
strain value.
The $\bar{F}$ approach does not alter the weak form residual in the kernel.  However, this change in the definition 
of the strain does affect the Jacobian calculated in the
[total Lagrangian](kernels/lagrangian/TotalLagrangianStressDivergence.md) and
[updated Lagrangian](/UpdatedLagrangianStressDivergence.md) kernels, as detailed in their documentation.

!alert warning
The $\bar{F}$ stabilization triggered by the `stabilize_strain` flag should only used for linear quad and hex elements.
$\bar{F}$ is unnecessary for higher order elements and ineffective for linear triangle and tetrahedral elements. 

## Cook's Membrane: A Demonstration the Stabilization is Effective

[cooksetup] shows *Cook's Membrane*, a classical problem for demonstrating volumetric locking and assessing stabilization
techniques for overcoming it.
When this problem is solved with a nearly a incompressible material it induces locking in unstabilized, linear, Q4 quad elements.
[smallcook] and [largecook] show the problem solved twice, first with small deformation kinematics and a linear elastic material
defined by $E=250$ and $\nu=0.4999999$ and then again with large deformation kinematics and a Neohookean material with $\lambda=416666611.0991259$ and 
$\mu=8300.33333888888926$.
Each plot shows the displacement at the tip of the beam as a function of mesh refinement

!media tensor_mechanics/cooksetup.png
       id=cooksetup
       style=width:30%;float:center;padding-top:1.5%;
       caption=Cook's membrane: a reference problem for testing locking and stabilization strategies

These plots demonstrate 

1. The problem with locking in both large and small deformations for unstabilized, linear elements.  The beam tip displacement
   is much smaller in the unstabilized problems compared to the true solution and the stabilized solutions (i.e. these elements
   are very stiff).  Moreover, mesh refinement is not effective at resolving the issue.
2. The $\bar{F}$ stabilization implemented in the Lagrangian kernel system effectively eliminates volumetric locking for
   both the updated and total Lagrangian formulations and for both small and large deformations.  The stabilized solutions
   for each kernel type are identical, they demonstrate the proper, non-locking stiffness, and mesh refinement converges the
   problem to a stable, analytic solution.

!media tensor_mechanics/smallcook.png
       id=smallcook
       style=width:50%;float:center;padding-top:1.5%;
       caption=Demonstration of $\bar{F}$ stabilization on a small deformation problem.

!media tensor_mechanics/largecook.png
       id=largecook
       style=width:50%;float:center;padding-top:1.5%;
       caption=Demonstration of $\bar{F}$ stabilization on a large deformation problem.

!bibtex bibliography

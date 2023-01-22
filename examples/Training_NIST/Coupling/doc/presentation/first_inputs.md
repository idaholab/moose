# Building a Simple Input File

!---

## MOOSE, a Partial Differential Equation Solver

- In thermomechanics we are mainly interested in solving heat transfer and
  solid mechanics.

!equation
-\nabla \cdot k \nabla T= 0

!equation
\nabla \cdot \sigma = 0

- For these problems, MOOSE uses the finite element method (FEM).

- FEM converts complex PDEs into a set of coupled algebraic equations that can be readily solved on a computer.

- FEM can represent problems with arbitrary geometry.

!---

## FEM Vocabulary

The following list contains terms commonly used when discussing the
finite element approach. These definitions are +NOT COMPREHENSIVE+. This
list is just to get the conversation started.

- Domain - The space or geometry of your problem.

- Element - To obtain the approximate solution, the domain must be
  subdivided (discretized) into simpler, smaller regions called elements.

- Node - The points at which the elements are connected. We typically
  compute the value of primary solution variables (temperature,
  displacement) at nodes. They are also where Dirichlet boundary conditions are
  applied.

- Boundary Condition - A constraint, or "load", applied to
  the domain.

- Quadrature Point - One step to finding the approximate
  solution to the PDE is integration. Quadrature points are where this
  integration happens. They are located within the elements.

- Test or Shape Function - Functions that help form the approximate
  solution to the PDE.

!---

## Module Heat Conduction

- MOOSE module is built to help solve:

  !equation
  \rho C_p\frac{\partial T}{\partial t} - \nabla\cdot k\nabla T - q = 0

  where $\rho$ is the mass density, $C_p$ is the specific heat, $T$ is
  the temperature, $k$ is the thermal conductivity, and $q$ is the
  volumetric heat generation rate.

- MOOSE provides spherically symmetric 1D, axisymmetric 2D,
  and 3D formulations supporting a wide range of [elements](https://libmesh.github.io/doxygen/classlibMesh_1_1Elem.html).

!---

## Module Heat Conduction (cont.)

!equation
\begin{aligned}
\rho C_p\frac{\partial T}{\partial t} - \nabla\cdot k\nabla T - q &= 0\\
T|_{\partial\Omega_1} &= g_1\\
\nabla T\cdot \hat{\bm{n}} |_{\partial\Omega_2} &= g_2
\end{aligned}

- Multiply by test function, integrate

!equation
\left(\psi_i, \rho C_p\frac{\partial T}{\partial t}\right) - (\psi_i, \nabla\cdot k\nabla T) - (\psi_i, q) = 0

- Integrate by parts

!equation
\left(\psi_i, \rho C_p\frac{\partial T}{\partial t}\right) + (\nabla \psi_i, k \nabla T) - (\psi_i, q) - \langle \psi_i, g_2 \rangle = 0

- Jacobian

!equation
\left(\psi_i, \rho C_p\left(\frac{\delta}{\delta t}\right)\phi_j\right) + \left(\nabla \psi_i, \nabla k\phi_j\right)

!---

## Required Blocks for the Input File

- `[Mesh]` - domain of the problem

- `[Variables]` - temperature and displacement

- `[Kernels]` - heat conduction, solid mechanics

- `[Materials]` - used by kernels, e.g. thermal conductivity

- `[BCs]` - specify Dirichlet or Neumann

- `[Executioner]`- steady state or transient

- `[Outputs]` - set options for how you want the output to look

!---

## Example Input File

The following input file is an example of how to solve the heat
conduction equation with a source term.

!row!
!col! width=45%

!style! fontsize=80%

!listing heat_cond_simmple.i
         block=Mesh Variables Kernels Materials

!style-end!

!col-end!

!col! width=5%
!col width=5%

$~$

!col-end!

!col! width=45%
!col width=45%

!style! fontsize=80%

!listing heat_cond_simmple.i
         block=BCs Executioner Outputs Postprocessors

!style-end!

!col-end!

!row-end!

!---

## [Mesh](https://mooseframework.inl.gov/syntax/Mesh/) Block

- The FEM mesh is defined in the `Mesh` block.

- A mesh can be read in from a file. There are many accepted formats
  (see the MOOSE manual). We typically use the Exodus file format and
  create meshes with CUBIT.

- Simple meshes can also be generated within the input file. We'll use
  this approach for our first examples.

- The sides are named in a logical way (bottom,
  top, left, right, front, and back).

!row!

!col! width=45%

!listing heat_cond_simmple.i
         block=Mesh

!col-end!

!col! width=5%
!col width=5%

$~$

!col-end!

!col! width=45%
!col width=45%

!media 10x10mesh.png style=width:80%

!col-end!

!row-end!

!---

## [Variables](https://mooseframework.inl.gov/syntax/Variables/) Block

- The primary or dependent variables in the PDEs (temperature,
  displacement) are defined in the `Variables` block.

- A user-selected unique name is assigned for each variable.

!listing heat_cond_simmple.i
         block=Variables

!---

## [Kernels](https://mooseframework.inl.gov/moose/syntax/Kernels/) Block

- The kernels (individual terms in the PDEs being solved) are listed
  in the `Kernels` block.

- Each kernel is assigned a specific variable (in this case temperature).

$~$

!equation
-\nabla\cdot k\nabla T - q = 0

$~$

!listing heat_cond_simmple.i
         block=Kernels

!---

## [Materials](https://mooseframework.inl.gov/moose/syntax/Materials/) Block

- Material properties are defined in the `Materials`
  block. Information from the materials block is used by some kernels.

- Here, thermal conductivity is defined to for use by the
  `HeatConduction` kernel.

!listing heat_cond_simmple.i
         block=Materials

!---

## [Boundary Conditions](https://mooseframework.inl.gov/syntax/BCs/) (BCs) Block

Define temperature on boundary

- Boundary conditions are defined in the `BCs` block.

- Many types of boundary conditions may be applied.

- For this simple example, the temperature is set on the left and
  right sides of the domain.

!listing heat_cond_simmple.i
         block=BCs

!---

## [Executioner](https://mooseframework.inl.gov/syntax/Executioner/) Block

- The `Executioner` block defines how the problem is solved.

- The parameters `solve_type` and `petsc_options` will be discussed
  later.

!listing heat_cond_simmple.i
         block=Executioner

!---

## [Outputs](https://mooseframework.inl.gov/syntax/Outputs/index.html) Block

- The results you will output from your simulation are defined in the
  `Outputs` block.

- This includes defining the file type (Exodus file here).

- Performance logs are also defined.

!listing heat_cond_simmple.i
         block=Outputs

!---

## [Postprocessors](https://mooseframework.inl.gov/syntax/Postprocessors/index.html) Block

- Analysis results in the form of single scalar values are defined in
  the `Postprocessors` block.

- May operate on elements, nodes, or sides of the model.

- Examples include `NodalExtremeValue`, `AverageElementSize`, and
  `SideAverageValue`.

- Documentation of Postprocessors will be shown later.

!listing heat_cond_simmple.i
         block=Postprocessors

!---

## Run Problem

- The problems shown here can be run either with an application such
  as BISON or BLUE_CRAB that links in the "heat_conduction" module, or with the MOOSE
  combined module executable.

- To run with the MOOSE combined modules executable, run:

  ```bash
  ~/projects/moose/modules/combined/combined-opt -i heat_cond_simmple.i
  ```

- To run with an application (BISON example shown here), run:

  ```bash
  ~/projects/bison/bison-opt -i heat_cond_simmple.i
  ```

- These examples assume your code is in the `~/projects`
  directory. Substitute in an appropriate path if it is located
  elsewhere.

!---

## Heat Conduction with Source: Results

$~$

!row!
!col! width=15%

$~$

!col-end!

!col! width=80%
!col width=80%

!media heat_conduction_with_source.png

!col-end!

!row-end!

!---

## Mechanics

<!-- - Use "Tensor Mechanics" and +not+ "Solid Mechanics" blocks. -->

- MOOSE modules' mechanics routines are built to help solve:

  !equation
  \nabla\cdot{\sigma} + b = 0

  where $\sigma$ is the stress and $b$ is a body force.

- MOOSE modules also supplies boundary conditions useful for
  mechanics (such as pressure).

- MOOSE modules provides spherically symmetric 1D, axisymmetric 2D
  (typically linear), and 3D fully nonlinear formulations. Either
  first or second order elements may be used (QUAD4 or QUAD8 for RZ;
  HEX8 or HEX20 for 3D).

!---

## Mechanics (cont.)

!equation
\begin{aligned}
\nabla\cdot{\sigma} + b &= 0\\
u|_{\partial\Omega_1} &= g_1\\
\sigma\cdot\hat{\bm{n}}|_{\partial\Omega_2} &= g_2
\end{aligned}

- Multiply by test function, integrate

!equation
(\psi_i,\nabla\cdot{\boldsymbol\sigma}) + (\psi_i, b) = 0

- Integrate by parts

!equation
-(\nabla\psi_i,{\boldsymbol\sigma}) + (\psi_i, b) + \langle \psi_i, g_2 \rangle = 0

- $\sigma = \bm{C}{\epsilon}$

!equation
-(\nabla\psi_i,\bm{C}{\boldsymbol\epsilon}) + (\psi_i, b) + \langle \psi_i, g_2 \rangle = 0

!---

## Mechanics: Spherically Symmetric 1D

- The 1D, 2D, and 3D classes have much in common.

- The calculation of the strain is of course different for the three
  formulations. However, they share material models.

- The spherically symmetric 1D strain is

!equation
\begin{aligned}
\epsilon_{rr} &= u_{r,r}\\
\epsilon_{zz} &= u_r / r\\
\epsilon_{\theta\theta} &= u_r / r\\
\end{aligned}

- The mesh for spherically symmetric 1D is defined such that the x-coordinate
  corresponds to the radial direction.

- No displacement in the x (radial) direction must be explicitly
  enforced in the input file for nodes at $x=0$.

!---

## Mechanics: Axisymmetric 2D

- The axisymmetric 2D strain is

!equation
\begin{aligned}
\epsilon_{rr} &= u_{r,r}\\
\epsilon_{zz} &= u_{z,z}\\
\epsilon_{\theta\theta} &= u_r / r\\
\epsilon_{rz} &= (u_{r,z} + u_{z,r}) / 2\\
\end{aligned}

- The mesh for RZ is defined such that the x-coordinate corresponds to
  the radial direction and the y-coordinate with the axial direction.

- No displacement in the x (radial) direction must be explicitly
  enforced in the input file for nodes at $x=0$.

!---

## Mechanics: Nonlinear 3D

!row!
!col! width=50%

!equation
F=\nabla x=\nabla u + I = RU

- The nonlinear kinematics formulation in MOOSE modules accommodates
  both large strains and large rotations.

- The deformation gradient $F$ can be viewed as the derivative of the
  current coordinates with respect to the original coordinates.

- $F$ can be decomposed into pure rotation $R$ and pure stretch $U$.

!col-end!

!col width=50%
!media deformationGradient.png

!row-end!

!---

## Mechanics: 3D

- We begin with a complete set of data for step $n$ and seek the
  displacements and stresses at step $n+1$. We first compute an
  incremental deformation gradient;

!equation
\mathbf{\hat{F}} = \frac{\partial{x^{n+1}}}{\partial{x^n}}

- With $\mathbf{\hat{F}}$, we next compute a strain increment that
  represents the rotation-free deformation from the configuration at
  $n$ to the configuration at $n+1$. We seek
  the stretching rate $\mathbf{D}$:

!equation
\begin{aligned}
\mathbf{D} =& \frac{1}{\Delta t}\operatorname{log}(\mathbf{\hat{U}})\\
=&\frac{1}{\Delta t}\operatorname{log}\left( \operatorname{sqrt}\left(\mathbf{\hat{F}}^T\mathbf{\hat{F}}\right)\right)\\
=&\frac{1}{\Delta t}\text{log}\left( \operatorname{sqrt}\left(\mathbf{\hat{C}}\right)\right)
\end{aligned}

- Here, $\mathbf{\hat{U}}$ is the incremental stretch tensor, and
  $\mathbf{\hat{C}}$ is the incremental Green deformation
  tensor. Through a Taylor series expansion, this can be determined in
  a straightforward, efficient manner.

!---

## Mechanics: 3D (cont.)

- $\mathbf{D}$ is passed to the constitutive model as an input for
  computing $\boldsymbol{\sigma}$ at $n+1$.

- The next step is computing the incremental rotation,
  $\mathbf{\hat{R}}$, where $\mathbf{\hat{F}}=
  \mathbf{\hat{R}}\mathbf{\hat{U}}$. Like for $\mathbf{D}$, an
  efficient algorithm exists for computing $\mathbf{\hat{R}}$. It is
  also possible to compute these quantities using an
  eigenvalue/eigenvector routine.

- With $\boldsymbol{\sigma}$ and $\mathbf{\hat{R}}$, we rotate the
  stress to the current configuration.

!equation
\begin{aligned}
\mathbf{\hat{F}} &= f(x)\\
\mathbf{D} &= f(\mathbf{\hat{F}})\\
\mathbf{\hat{R}} &= f(\mathbf{\hat{F}})\\
\Delta\boldsymbol{\sigma} &= f(\mathbf{D, \boldsymbol{\sigma}_n})\\
\boldsymbol{\sigma}_{n+1} &= \mathbf{\hat{R}}(\boldsymbol{\sigma}_n + \Delta\boldsymbol\sigma)\mathbf{\hat{R}}^T
\end{aligned}

!---

## Mechanics: Material Models

- The material models for 1D, axisymmetric 2D, and 3D are formulated
  in an incremental fashion (think hypo-elastic).

- Thus, the stress at the new step is the old stress plus a stress
  increment:

!equation
\boldsymbol{\sigma}_{n+1} = \boldsymbol{\sigma}_n + \Delta\boldsymbol\sigma.

- The incremental formulation is particularly useful for plasticity
  and creep models.

!---

## Let's add some more physics... Mechanics!

The following blocks have to be added or modified to our input file if
we want to include mechanics behavior.

!row!
!col! width=45%

!style! fontsize=80%

!listing heat_cond_solid_mech.i
         block=Variables Modules/TensorMechanics/Master Kernels

!style-end!

!col-end!

!col! width=5%
!col width=5%

$~$

!col-end!

!col! width=45%
!col width=45%

!style! fontsize=80%

!listing heat_cond_solid_mech.i
         block=Materials

!style-end!

!col-end!

!row-end!

!---

## Let's add some more physics... Mechanics!

The following blocks have to be added or modified to our input file if
we want to include mechanics behavior.

!listing heat_cond_solid_mech.i
         block=BCs

!---

## Heat Conduction + Mechanics: Results

$~$

!row!
!col! width=15%

$~$

!col-end!

!col! width=80%
!col width=80%

!media heat_cond_source_solid_mech.png

!col-end!

!row-end!

!---

## Modules: Contact - Finite Element Contact Basics

- A contact capability in a solid mechanics finite element code
  prevents the penetration of one domain into another, or part of one
  domain into itself.

!row!
!col! width=35%

$~$

!col-end!

!col! width=60%
!col width=60%

!media contact_basics.png

!col-end!

!row-end!

!---

## Modules: Contact - Required Capabilities

A necessary but insufficient list:

!style! halign=center
!row!
!col! small=12 medium=6 large=3 style=margin-left:auto;margin-right:5px;
- Search

  - Exterior identification

    - Nearby nodes

    - Capture box

    - Binary search, e.g.

  - Contact existence

    - More geometric work

      - Penetration point
!col-end!

!col! small=12 medium=6 large=3 style=margin-right:2%;
- Enforcement

  - Formulation of contact force

  - Formulation of Jacobian

  - Interaction with other capabilities

!col-end!
!row-end!
!style-end!

!---

## Contact: Overview

!row!
!col! width=50%

- In node-face contact, nodes (green) may not penetrate faces (defined
  by orange nodes).

- Forces must be determined to push against the two contacting bodies.

- No force should be applied where the bodies are not in contact.

- The contact forces must increase from zero as the bodies first come
  into contact.

!col-end!

!col! width=50%
!col width=50%

!media contact.png

!col-end!

!row-end!

!---

## Contact: Constraints

- $g\le 0$; the gap (penetration distance) must be non-positive

- $t_N\ge 0$; the contact force must push bodies apart

- $t_Ng=0$; the contact force must be zero if the bodies are not in
  contact

- $t_N\dot{g}=0$; the contact force must be zero when constraints are
  formed and released

- The gap in the normal direction for constraint $i$ is ($n$ is the
  normal, $N$ denotes normal direction, $d_s$ is position of the secondary
  node, $d_c$ is position of the contact point, and $G$ is a matrix):

!row!
!col! width=60%

$~$

!equation
\begin{aligned}
g^i_N &= n^i(d^i(t)-d^i_c(t))\\
g^i_N &= G^i_N(d(t))
\end{aligned}

!col-end!

!col! width=40%
!col width=40%

!media contact_constraints.png

!col-end!

!row-end!

!---

## Contact: Contact Options

!style! fontsize=75%

- `formulation: kinematic` or `penalty`

  - Kinematic is more accurate but also harder to solve.

- `model:frictionless, glued`, or `coulomb`

  - Frictionless enforces the normal constraint and allows nodes to
    come out of contact if they are in tension. Glued ties nodes where
    they come into contact with no release. Coulomb is frictional
    contact with release.

- `friction_coefficient`

  - Coulomb friction coefficient.

- `penalty`

  - The penalty stiffness to be used in the constraint.

- `primary`

  - The surface corresponding to the faces in the constraint.

- `secondary`

  - The surface corresponding to the nodes in the constraint.

- `normal_smoothing_distance`

  - Distance from face edge in parametric coordinates over which to
    smooth the normal. Helps with convergence. Try 0.1.

- `tension_release`

  - The tension value that will allow nodes to be released. Defaults
    to zero.

!style-end!

!---

## Even more physics... CONTACT

The following blocks have to be added or modified to our input file if
we want to include the effects of mechanical contact.

!listing heat_cond_solid_mech_contact_therm_mg.i
         block=Contact

!---

## Heat Conduction + Mechanics + Contact: Results

!row!
!col! width=50%

- q = 600

- Bottom block heats and expands upward, but is not yet in contact

- Blocks do not communicate thermally (no gap heat transfer)

!col-end!

!col! width=50%
!col width=50%

!media mech_contact_temp_time1.png

!col-end!

!row-end!

!---

## Heat Conduction + Mechanics + Contact: Results (cont.)

!row!
!col! width=50%

- q = 600

- Bottom block heats and expands upward, but is not yet in contact

- Vertical displacement plots show curvature of top surface

!col-end!

!col! width=50%
!col width=50%

!media mech_contact_disp_y_time1.png

!col-end!

!row-end!

!---

## Heat Conduction + Mechanics + Contact: Results (cont.)

!row!
!col! width=50%

- q = 1500

- Further heating and upward expansion brings blocks into contact;
  first at the center where the bottom block is hottest

- Still, blocks do not communicate thermally (no gap heat transfer)

!col-end!

!col! width=50%
!col width=50%

!media mech_contact_temp_time2.png

!col-end!

!row-end!

!---

## Heat Conduction + Mechanics + Contact: Results (cont.)

!row!
!col! width=50%

- q = 1500

- Contour scale is set to show displacement in top block resulting
  from mechanical contact

!col-end!

!col! width=50%
!col width=50%

!media mech_contact_disp_y_time2.png

!col-end!

!row-end!

!---

## Modules: Heat Conduction: Gap Heat Transfer

- The principle is that the heat leaving one body must equal that entering another. For bodies $i$ and $j$ with heat transfer surface $\Gamma$:

!equation
\int_{\Gamma_i} h \Delta T dA_i = \int_{\Gamma_j} h \Delta T dA_j

- Gap heat transfer is modeled using the relation:

  !equation
  h_{gap} = h_g + h_s + h_r

  where $h_{gap}$ is the total conductance across the gap, $h_g$ is
  the gas conductance, $h_s$ is the increased conductance due to
  solid-solid contact, and $h_r$ is the conductance due to radiant
  heat transfer.

- In MOOSE modules, only the gas conductance $h_g$ is active by
  default.

- The form of $h_g$ in MOOSE modules is:

  !equation
  h_g = \frac{k_g}{d_g}

  where $k_g$ is the conductivity in the gap and $d_g$ is the gap
  distance.

!---

## Adding Thermal Contact

!row!
!col! width=45%

!listing heat_cond_solid_mech_contact_therm_mg.i
         block=MortarGapHeatTransfer

$~$

!equation
h = \frac{k_g}{d_g}

!col-end!

!col! width=10%
!col width=10%

$~$

!col-end!

!col! width=45%
!col width=45%

!media gap.png

!col-end!

!row-end!

!---

## Heat Conduction + Mechanics + Contact + Thermal Contact: Results

!row!
!col! width=50%

- q = 750

- Heat tranfer occurs through the gap medium prior to mechanical
  contact

!col-end!

!col! width=50%
!col width=50%

!media mech_therm_contact_time1.png

!col-end!

!row-end!

!---

## Heat Conduction + Mechanics + Contact + Thermal Contact: Results

!row!
!col! width=50%

- q = 1330

- Combined thermal and mechanical contact

!col-end!

!col! width=50%
!col width=50%

!media mech_therm_contact_time2.png

!col-end!

!row-end!

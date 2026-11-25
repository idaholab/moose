# DSG shell elements

The discrete shear gap (DSG) shell family is a set of four flat shell elements for thin and moderately thick plates and shells: the three-node and six-node triangles and the four-node and eight-node quadrilaterals (`TRI3`, `TRI6`, `QUAD4` and `QUAD8` meshes). Each node carries six degrees of freedom, three displacements and three rotations about the global axes. The elements combine a plane stress membrane with a Reissner-Mindlin plate whose transverse shear strains come from the discrete shear gaps of [!cite](bletzinger2000unified), which remove shear locking without a reduced or selective integration. The family complements the four-node shell of [ShellElements](/ShellElements.md): it offers quadratic members and a drilling rotation, and it is restricted to small displacements and a linear elastic section (see [Limitations](#limitations) below).

The implementation follows the pattern of the other elements of the module. The kinematics live in a strain material, the constitutive law in two materials and the equilibrium in kernels:

| Object | Role |
|---|---|
| [ComputeDSGShellStrain](/ComputeDSGShellStrain.md) | element frame, strain-displacement matrix, generalized strains |
| [ComputeDSGShellSectionStiffness](/ComputeDSGShellSectionStiffness.md) | section stiffness of the linear elastic plate |
| [ComputeDSGShellResultants](/ComputeDSGShellResultants.md) | stress resultants |
| [StressDivergenceDSGShell](/StressDivergenceDSGShell.md) | internal force and its Jacobian, one kernel per degree of freedom |
| [InertialForceDSGShell](/InertialForceDSGShell.md) | inertial force, with Rayleigh damping and the HHT damping term |
| [DSGShellMassEigenKernel](/DSGShellMassEigenKernel.md) | mass matrix for eigenvalue solves |
| [DistributedLoadDSGShell](/DistributedLoadDSGShell.md) | load per unit area along an axis or along the normal |
| [DSGShellResultantsAux](/DSGShellResultantsAux.md) | a resultant written to an elemental auxiliary variable |
| [DSGShellNodalResultants](/DSGShellNodalResultants.md) | resultants recovered at the nodes |
| [DSGShellNodalResultantsAux](/DSGShellNodalResultantsAux.md) | a nodal resultant written to a nodal auxiliary variable |

The input files of the tests in `modules/solid_mechanics/test/tests/shell_dsg` show the complete setup. The verification cases are collected in [dsg_shell_vandv](/dsg_shell_vandv.md).

## Element frame

Every element is flat ([fig_dsg_shell]). Its normal $\mathbf{e}_3$ is the normalized cross product of the two diagonals of a quadrilateral, or of the two edges leaving the first node of a triangle. The first in-plane axis $\mathbf{e}_1$ is the projection on the element plane of the reference direction given by `reference_first_local_direction` (the global X axis by default, as for the four-node shell); when that direction is normal to the element (within 0.05 degrees) the global axis least aligned with the normal is projected instead. The second axis closes the right-handed frame, $\mathbf{e}_2 = \mathbf{e}_3 \times \mathbf{e}_1$. The rotation matrix $\mathbf{R}$ with these three vectors as rows takes a global vector to the element frame, and the nodal degrees of freedom are rotated triplet by triplet,

\begin{equation}
\mathbf{u}_{\mathrm{local}} = \mathbf{T} \mathbf{u}, \qquad \mathbf{T} = \mathrm{diag}(\mathbf{R}, \mathbf{R}, \dots, \mathbf{R}).
\end{equation}

The in-plane coordinates $(x, y)$ of the nodes are measured in this frame from the corner centroid of a quadrilateral or from the first node of a triangle. The choice of origin changes no strain. Element quantities that are reported in the element frame, the resultants above all, follow these axes: a plate in the XY plane with the default reference direction has $\mathbf{e}_1$ along X and $\mathbf{e}_2$ along Y.

!media media/solid_mechanics/dsg_shell_element.png
      style=width:100%;
      id=fig_dsg_shell
      caption=The element frame and the gap lines of the four-node (A) and three-node (B) elements, and the sign of a positive bending moment (C): tension on the face the normal $\mathbf{e}_3$ points to.

## Kinematics

The in-plane displacements $u, v$, the transverse displacement $w$ and the rotations $\theta_1, \theta_2, \theta_3$ about the local axes are interpolated with the shape functions $N_k$ of the element: linear and quadratic on the triangles, bilinear and serendipity on the quadrilaterals. A point at the distance $z$ from the mid-surface moves by $z \theta_2$ along $\mathbf{e}_1$ and by $-z \theta_1$ along $\mathbf{e}_2$, so the transverse shear strains of the plate are $\gamma_1 = w_{,1} + \theta_2$ and $\gamma_2 = w_{,2} - \theta_1$. The quantities $\beta_1 = -\theta_2$ and $\beta_2 = \theta_1$ that these expressions subtract from the slopes are the ones the shear gaps work with: $\gamma_s = w_{,s} - \beta_s$.

The generalized strains are collected in a vector of nine components,

\begin{equation}
\boldsymbol{\varepsilon} = \left[ \varepsilon_{11}, \varepsilon_{22}, 2\varepsilon_{12}, \chi_{11}, \chi_{22}, 2\chi_{12}, \gamma_1, \gamma_2, \delta \right]^T = \mathbf{B} \mathbf{u},
\end{equation}

built by the strain-displacement matrix $\mathbf{B}$ of the whole element in global degrees of freedom:

- The membrane strains are the derivatives of the in-plane displacements, $\varepsilon_{11} = u_{,1}$, $\varepsilon_{22} = v_{,2}$ and $2\varepsilon_{12} = u_{,2} + v_{,1}$.
- The curvatures are the derivatives of the displacement of the normal, $\chi_{11} = \theta_{2,1}$, $\chi_{22} = -\theta_{1,2}$ and $2\chi_{12} = \theta_{2,2} - \theta_{1,1}$. With this sign a positive curvature gives a positive moment, the integral of the stress times the distance from the mid-surface, $M = \int \sigma z \, \mathrm{d}z$, positive when the face the normal $\mathbf{e}_3$ points to is in tension. This is the convention of [ShellResultantsAux](/ShellResultantsAux.md) of the four-node shell, and of every moment reported by the family.
- The transverse shear strains come from the discrete shear gaps described next.
- The drilling strain is the difference between the drilling rotation and the rotation of the in-plane displacement field, $\delta = \theta_3 - \tfrac{1}{2}(v_{,1} - u_{,2})$. It vanishes for a true rotation of the element and ties the sixth degree of freedom to the membrane field.

### Discrete shear gaps

In a displacement-based Reissner-Mindlin element the transverse shear strain $\gamma_s = w_{,s} - \beta_s$ along a direction $s$ does not vanish in pure bending when the interpolations of $w$ and of the rotations are of the same order, and the element locks as the plate becomes thin. The discrete shear gap method of [!cite](bletzinger2000unified) replaces the pointwise shear strain by the derivative of a *shear gap* field. The shear gap at a node $k$ along a parent direction is the change of the transverse displacement between a reference node $r$ and node $k$ minus the integral of $\boldsymbol{\beta}$ along the line that joins them,

\begin{equation}
\Delta w_k = w_k - w_r - \int_r^k \boldsymbol{\beta} \cdot \mathrm{d}\mathbf{x} .
\end{equation}

In pure bending the integral of $\boldsymbol{\beta}$ reproduces the change of $w$ exactly and the gap vanishes, so the element cannot lock. The gaps are interpolated with the shape functions of the element and differentiated along the parent directions $\xi$ and $\eta$; the derivatives are mapped to the local axes with the inverse Jacobian of the element:

\begin{equation}
\begin{bmatrix} \gamma_1 \\ \gamma_2 \end{bmatrix} = \mathbf{J}^{-1} \begin{bmatrix} \sum_k N_{k,\xi} \, \Delta w_k^{\xi} \\ \sum_k N_{k,\eta} \, \Delta w_k^{\eta} \end{bmatrix} .
\end{equation}

The lines along which the gaps are taken are fixed per member: the three-node element has a single linear gap field measured from its first node in both directions, the four-node element measures the gap along each of its four edges in the direction of that edge (the DSG4 of [!cite](bletzinger2000unified)), and the quadratic elements measure it along the lines of nodes of the parent element, corner to midside and midside to corner. The line integral of $\boldsymbol{\beta}$ is the trapezoidal rule on the linear elements (exact for their linear field) and a two-point Gauss rule along the straight parent line on the quadratic ones (exact for their cubic integrand). Since the gaps are linear in the nodal degrees of freedom, they are assembled once per element as two gap matrices, one per parent direction, and the shear rows of $\mathbf{B}$ at a quadrature point are the mapped derivatives of those matrices.

The three-node element has only three curvature and two shear strain components for its six plate degrees of freedom and remains stiff in bending for very thin plates. Its transverse shear stiffness therefore carries the factor of [!cite](lyly1993stable),

\begin{equation}
\frac{t^2}{t^2 + \alpha h^2}, \qquad \alpha = 0.15,
\end{equation}

with $h$ the longest edge of the element, which relaxes the shear constraint as the mesh becomes coarse relative to the thickness. The other three members do not need it.

## Section stiffness and resultants

The resultants are the section stiffness times the generalized strains, $\boldsymbol{\sigma} = \mathbf{D} \boldsymbol{\varepsilon}$, with

\begin{equation}
\boldsymbol{\sigma} = \left[ N_{11}, N_{22}, N_{12}, M_{11}, M_{22}, M_{12}, Q_1, Q_2, M_d \right]^T ,
\end{equation}

the membrane forces, the bending moments, the transverse shear forces and the drilling moment, all per unit length and in the element frame. For a homogeneous isotropic plate of thickness $t$, Young's modulus $E$ and Poisson's ratio $\nu$ the matrix is block diagonal,

\begin{equation}
\mathbf{D} = \mathrm{diag}\left( E t \, \mathbf{P}, \; \frac{E t^3}{12} \mathbf{P}, \; \kappa G t \, \mathbf{I}_2, \; G t \right), \qquad
\mathbf{P} = \frac{1}{1 - \nu^2} \begin{bmatrix} 1 & \nu & 0 \\ \nu & 1 & 0 \\ 0 & 0 & \tfrac{1 - \nu}{2} \end{bmatrix},
\end{equation}

with $G = E / (2 (1 + \nu))$ and the shear correction factor $\kappa = 5/6$. The drilling stiffness $G t$ is the in-plane shear stiffness of the section, the consistent penalty on the drilling strain. A positive bending moment puts the face the normal $\mathbf{e}_3$ points to in tension, as stated with the curvatures; $M_{11}$ is the moment of the stresses in the first direction, which bends the plate about the second axis, and $M_{22}$ the moment of the stresses in the second direction. The drilling stiffness can be scaled with `drilling_stiffness_factor` on membrane-dominated problems with coarse or distorted meshes.

## Equilibrium

The internal force on the degrees of freedom of the element is $\mathbf{B}^T \boldsymbol{\sigma}$ integrated over the element, and the stiffness $\mathbf{B}^T \mathbf{D} \mathbf{B}$. Both are evaluated with the quadrature rule of the element, which for Lagrange variables of the element's order is three points on the three-node triangle, two by two on the four-node quadrilateral, six points on the six-node triangle and three by three on the eight-node quadrilateral. The kernel [StressDivergenceDSGShell](/StressDivergenceDSGShell.md) assembles one degree of freedom at a time, with the exact Jacobian written out from the stored matrices, and the strain material maps every quadrature point back to the parent element, so an evaluation on an edge of the element builds the same matrices as an evaluation inside it.

## Inertia

The kinetic energy of the shell mid-surface gives a consistent translational mass,

\begin{equation}
M_{kl} = \int_A \rho t \, N_k N_l \, \mathrm{d}A ,
\end{equation}

on each of the three displacements, and a rotary inertia $\rho t^3 / 12$ on the two rotations in the plane of the element. In the global frame the rotary inertia acts through the projector $\mathbf{I} - \mathbf{e}_3 \mathbf{e}_3^T$, which couples the three rotation variables and leaves the drilling rotation without inertia. [InertialForceDSGShell](/InertialForceDSGShell.md) computes these forces, with mass-proportional Rayleigh damping and the HHT shift of the damping term as [InertialForce](/InertialForce.md) does; the rotary inertia can be turned off. The velocities and accelerations are those of the [NewmarkBeta](/NewmarkBeta.md) time integrator.

### Natural frequencies

The same mass, written as [DSGShellMassEigenKernel](/DSGShellMassEigenKernel.md) with the eigen vector tag, gives the generalized eigenvalue problem $\mathbf{K} \mathbf{u} = \omega^2 \mathbf{M} \mathbf{u}$ for the [Eigenvalue](/Eigenvalue.md) executioner. The mass matrix is singular on the drilling rotation, which carries no inertia; the shift-and-invert transformation of the solver handles the infinite eigenvalues that come with it, and the massless rotations are condensed exactly on the displacements. The input needs an [EigenDirichletBC](/EigenDirichletBC.md) beside the Dirichlet condition of every supported variable and a preconditioner with the full coupling between the displacements and the rotations, as the example of the kernel page shows.

## Using the elements

An input needs the six variables, six stress divergence kernels (one per `component`), the three materials, and for dynamics six inertia kernels with a `density` material property. The quadratic members need a second-order mesh and `SECOND` order variables. Loads per unit area go through [DistributedLoadDSGShell](/DistributedLoadDSGShell.md), with the sign convention of the module's pressure loads (a positive value acts against the axis or against the normal); nodal forces and moments through nodal kernels on the displacement and rotation variables; supports through Dirichlet conditions on the six variables. The resultants are read with [DSGShellResultantsAux](/DSGShellResultantsAux.md), one elemental variable per resultant. For smooth plots and for values at the nodes, the strain material evaluates the strains at the nodes of each element when asked (`compute_nodal_strains`), [DSGShellNodalResultants](/DSGShellNodalResultants.md) averages the resultants of the elements sharing each node, and [DSGShellNodalResultantsAux](/DSGShellNodalResultantsAux.md) writes them to nodal variables.

!listing modules/solid_mechanics/test/tests/shell_dsg/static/cantilever_strip.i block=Kernels Materials

## Limitations id=limitations

- +Small displacements and rotations.+ The strain material builds the element frame and the strain-displacement matrix from the undeformed geometry, and the strains are linear in the degrees of freedom. There is no finite strain and no large rotation formulation, unlike [ADComputeFiniteShellStrain](/ADComputeFiniteShellStrain.md) of the four-node shell.
- +Flat elements.+ Every element is treated as flat, in the plane through the corner centroid of a quadrilateral or through the first node of a triangle. A curved surface is represented by facets, and a quadrilateral whose four corners are not coplanar is projected on its mean plane. Keep quadrilaterals flat on curved surfaces, or use triangles.
- +Linear elastic, isotropic, homogeneous section.+ The section stiffness is the one of a homogeneous isotropic plate of constant thickness. There is no layered section, no orthotropy and no inelastic section law.
- +Rotary inertia.+ The rotary inertia acts on the two rotations in the plane of the element and is zero on the drilling rotation, which is held by the drilling stiffness alone, in the transient and in the eigenvalue problem alike.
- +Jacobian.+ The stiffness is hand-written from the stored matrices rather than obtained by automatic differentiation: the element is linear in its degrees of freedom, so the product $\mathbf{B}^T \mathbf{D} \mathbf{B}$ is the exact Jacobian and the dual-number assembly of the four-node shell would cost more for the same matrix. The Jacobian testers of the battery check it on every member.
- +HHT time integration.+ The `alpha` parameter of the inertia kernel applies the HHT shift to the mass-proportional damping term only, as [InertialForce](/InertialForce.md) does; the stress divergence kernel has no `alpha`.
- +Variable order.+ The six variables must be Lagrange variables of the element's order, `FIRST` on `TRI3` and `QUAD4` and `SECOND` on `TRI6` and `QUAD8`. The quadratic members need a second-order mesh.
- +Other shell objects.+ The material properties of this family are its own (`dsg_shell_*`). The objects of the four-node shell ([ADStressDivergenceShell](/ADStressDivergenceShell.md), [ShellResultantsAux](/ShellResultantsAux.md), [ADDistributedLoadShell](/ADDistributedLoadShell.md) and the others) do not read them, and the two families are not mixed on one block.
- +Transverse shear of the three-node element.+ The thin-limit factor of the three-node element depends on its longest edge, so on a graded mesh its transverse shear stiffness varies from element to element and a constant shear state is reproduced exactly on a uniform mesh only. The other members carry no size-dependent factor.

!bibtex bibliography

# C0 Timoshenko Beam Element

A beam element [fig_beam] is used to model the response of a structural element that is long in one dimension compared to its cross-section. There are two popular formulation of beam elements:

!media media/tensor_mechanics/beam_representation.png
      style=width:50%;margin-left:2%;float:right
      id=fig_beam
      caption=Beam element with 2 nodes and 3 translational and 3 rotational degrees of freedom at each node.

1. +Euler-Bernoulli beam element+: used to model bending deformation in long and slender beams. The two main assumptions in this beam theory are that: (i) the beam cross-section is rigid and does not deform under the application of transverse or lateral loads, and (ii) the cross-section of the beam remains planar and normal to the deformed axis of the beam.

1. +Timoshenko beam element+ [!citep](timoshenko_correction_1921, timoshenko_transverse_1922): used to model both shear and bending deformation in short and thick beams. The beam cross-section does not deform in this beam theory as well and it remains planar. But the cross-section need not be normal to the deformed axis of the beam. The Euler-Bernoulli beam element can be derived as a special case of the Timoshenko beam element.

Therefore, a C0 Timoshenko beam element is implemented in MOOSE. This element has two nodes and each node has 6 degrees of freedom (DOFs) - 3 translational and 3 rotational displacements. All the 12 DOFs are considered to be independent and the variation of both translational and rotational displacements along the length of the beam are modeled using first order Lagrange shape functions. The independent rotational DOFs at the nodes makes it easier to model the shear deformation which results in non-perpendicular cross-sections with respect to the beam axis.

The basic equation of motion for a quasi-static beam is the same as that of a continuum brick element:
\begin{equation}
\nabla \cdot \sigma = F_{ext}
\end{equation}

An updated Lagrangian formulation similar to the one in [!cite](bathe_large_1979) is used in the calculation of beam stresses and strains.

## Strain Calculation

The first step to calculating stresses is to calculate the strains in the beam element. There are four main coordinate systems to consider - global coordinate system, original beam local coordinate system and beam local coordinate system at time $t$ and $t + \Delta t$. In the updated Lagrangian formulation, starting from the configuration at time $t$, the strain increment (which is a measure of the deformation of the beam) and rotation increment (which is a measure of the rigid rotation of the beam) are calculated to arrive at the state of the beam at $t + \Delta t$.

### Rotation increment

Let $^0R$ be the rotation matrix from the global coordinate system to the original beam local coordinate system. This is calculated using the unit vector along the axis of the beam (assumed to be the beam local x axis), the user-provided `y_orientation` that is orthonormal to the beam axis, and the cross product of the beam-axis and `y_orientation`. If the beam undergoes small deformation/rotation, the rotation matrix is not incrementally updated and the rotation matrix at $t+\Delta t$ ($^{t+\Delta t}R$) is assumed to be same as $^0R$.

If the beam undergoes finite deformation, a rotation increment ($\Delta R$) that transforms the beam from beam local configuration at $t$ to the beam local configuration at $t + \Delta t$ is calculated at each time step. This calculation is performed using Euler angles as described below.

Let $^{t+\Delta t} {u_1}^i$, $^{t+\Delta t} {u_2}^i$ and $^{t+\Delta t} {u_3}^i$ denote the translational displacements at node $i$ at $t+\Delta t$. These displacements are the difference between the nodal positions at time $t+\Delta t$ and $t$ and are in the global coordinate system. Here 1, 2 and 3 refers to the x, y and z coordinate, respectively. Similarly, let $^{t+\Delta t} {\theta_1}^i$, $^{t+\Delta t} {\theta_2}^i$ and $^{t+\Delta t} {\theta_3}^i$ denote the rotational displacements at node $i$ at $t+\Delta t$ in the global coordinate system. Let $^{t+\Delta t} \{ {\Delta u} \} = ^{t+\Delta t} {\{\Delta u\}}^1 - ^{t+\Delta t} {\{\Delta u\}}^0$ and $^{t+\Delta t} \{ {\Delta \theta} \} = ^{t+\Delta t} {\{\Delta \theta\}}^1 - ^{t+\Delta t} {\{\Delta \theta\}}^0$ be the differential displacements along the length of the beam.

These global displacements are converted to the beam local configuration at $t$ using the rotation matrix $^tR$ as follows:
\begin{equation}
^{t+\Delta t} {\{ {\Delta u} \}}^{local} = {}^tR \;\; ^{t+\Delta t} \{ {\Delta u} \}
\end{equation}

\begin{equation}
^{t+\Delta t} {\{ {\Delta \theta} \}}^{local} = {}^tR \;\; ^{t+\Delta t} \{ {\Delta \theta} \}
\end{equation}

These beam local displacements are then used in the calculation of the Euler angles $\alpha$, $\beta$ and $\gamma$ as in [!cite](bathe_large_1979). These Euler angles are used to calculate the rotation increment $\Delta R$. Using $\Delta R$, the rotation matrix at $t+\Delta t$ can be calculated as:
\begin{equation}
{}^{t+\Delta t}R = \Delta R \; {}^tR
\end{equation}

The rotation matrix is incrementally updated if [ComputeFiniteBeamStrain](/ComputeFiniteBeamStrain.md) is used. If [ComputeIncrementalBeamStrain](/ComputeIncrementalBeamStrain.md) is used, the rotation matrix is same as the initially computed rotation matrix ($^0R$).

### Strain increment

To calculate the beam strain increment, the incremental beam displacements at time $t+\Delta t$ in the local configuration at $t+\Delta t$ are obtained using ${}^{t+\Delta t} R$ to transform the displacements from global to local configuration at $t+\Delta t$. For simplifying notation, let ${u_{j}}^i$ and ${\theta_{j}}^i$ be the translational and rotational displacements at node $i$ at $t+\Delta t$ in the beam local configuration at $t+\Delta t$. Interpolating the nodal DOFs along the axis of the beam using first order Lagrange shape functions gives $u_{j}(x) = u_{j}^0 \frac{x}{{}^0L} + u_{j}^1 \frac{{}^0L-x}{{}^0L}$ and $\theta_{j}(x) = \theta_{j}^0 \frac{x}{{}^0L} + \theta_{j}^1 \frac{{}^0L-x}{{}^0L}$. However, $u_j(x)$ only gives the translational displacement of the beam axis. The translational displacement at any point on the beam is obtained as follows:
\begin{equation}
{u_1}(x,y,z) = u_1(x) - \theta_3(x) y + \theta_2(x) z
\end{equation}
\begin{equation}
{u_2}(x,y,z) = u_2(x) - \theta_1(x) z
\end{equation}
\begin{equation}
{u_3}(x,y,z) = u_3(x) + \theta_1(x) y
\end{equation}

The axial strain ($\epsilon_{11}$) and the shear strains ($\epsilon_{12}$ and $\epsilon_{13}$) are then obtained as follows:
\begin{equation}
\epsilon_{11}(x,y,z) = \frac{\partial u_1(x,y,z)}{\partial x} = \frac{\partial u_1(x)}{\partial x} - \frac{\partial \theta_3(x)}{\partial x} y + \frac{\partial \theta_2(x)}{\partial x} z
\end{equation}

\begin{equation}
\epsilon_{12}(x,y,z) = \frac{\partial u_1(x,y,z)}{\partial y} + \frac{\partial u_2(x,y,z)}{\partial x} = -\theta_3 + \frac{\partial u_2(x)}{\partial x} - \frac{\partial \theta_1(x)}{\partial x} z
\end{equation}

\begin{equation}
\epsilon_{13}(x,y,z) = \frac{\partial u_1(x,y,z)}{\partial z} + \frac{\partial u_3(x,y,z)}{\partial x} = \theta_2 + \frac{\partial u_3(x)}{\partial x} + \frac{\partial \theta_1(x)}{\partial x} y
\end{equation}

It should be noted here that using a linear interpolation of the rotational variables in the calculation of shear strain leads to shear locking of the beam. Using $\theta_1(x) = ({\theta_1}^0 + {\theta_1}^1)/2$ in the calculation of $\epsilon_{12}$ and $\epsilon_{13}$ ensures that the beam doesn't lock under shear deformation [!citep](prathap_reduced_1982).

The above translational strain increments are functions of x, y and z. However, since the beam cross-section does not deform, these strains can be integrated over the cross-section to obtain strain increments as a function of only x.
\begin{equation}
\epsilon_{1}(x) = \int_A \epsilon_{11}(x,y,z) dA= \frac{\partial u_1(x)}{\partial x} A - \frac{\partial \theta_3(x)}{\partial x} A_y + \frac{\partial \theta_2(x)}{\partial x} A_z
\end{equation}

\begin{equation}
\epsilon_{2}(x) = \int_A \epsilon_{12}(x,y,z) dA= -\theta_3 A + \frac{\partial u_2(x)}{\partial x} A - \frac{\partial \theta_1(x)}{\partial x} A_z
\end{equation}

\begin{equation}
\epsilon_{3}(x) = \int_A \epsilon_{13}(x,y,z) dA= \theta_2 A + \frac{\partial u_3(x)}{\partial x} A + \frac{\partial \theta_1(x)}{\partial x} A_y
\end{equation}

where $A$ is the cross-sectional area, $A_y = \int_A y dA$ and $A_z = \int_A z dA$. $\epsilon_{1}(x)$, $\epsilon_{2}(x)$ and $\epsilon_{3}(x)$ are the axial and shear increments.

Apart from the translational strain increments, rotational strain increments also need to be calculated.
\begin{equation}
\kappa_1(x) = \int_A (\epsilon_{13}(x,y,z) y - \epsilon_{12}(x,y,z) z) dA
            = \theta_2 A_y + \frac{\partial u_3(x)}{\partial x} A_y + \frac{\partial \theta_1(x)}{\partial x} I_x  + \theta_3 A_z - \frac{\partial u_2(x)}{\partial x} A_z
\end{equation}

\begin{equation}
\kappa_2(x) = \int_A \epsilon_{11}(x,y,z) z dA = \frac{\partial u_1(x)}{\partial x} A_z - \frac{\partial \theta_3(x)}{\partial x} Iyz + \frac{\partial \theta_2(x)}{\partial x} I_z
\end{equation}

\begin{equation}
\kappa_3(x) = -\int_A \epsilon_{11}(x,y,z) y dA = -\frac{\partial u_1(x)}{\partial x} A_y + \frac{\partial \theta_3(x)}{\partial x} I_y - \frac{\partial \theta_2(x)}{\partial x} Iyz
\end{equation}

where $I_y = \int_A y^2 dA$, $I_z = \int_A z^2 dA$, $I_x = \int_A (y^2 + z^2) dA$ and $Iyz = \int_A yz dA$. Note that $Iyz$ is assumed to be zero for simplicity and this assumption is valid for symmetric cross-sections such as square, rectangular and circular. $I_x$ is automatically calculated from $I_y$ and $I_z$ unless $I_x$ is provided as input by the user.

The above strain increments are calculated using the small strain assumption. If the `large_strain` option in [ComputeIncrementalBeamStrain](/ComputeIncrementalBeamStrain.md) or [ComputeFiniteBeamStrain](/ComputeFiniteBeamStrain.md) is set to `true`, then the strains are calculated using the equations below:

\begin{equation}
\epsilon_{11}(x,y,z) = \frac{\partial u_1(x,y,z)}{\partial x} + \frac{1}{2} {\left[ {\left( \frac{\partial u_1(x,y,z)}{\partial x}\right)}^2 + {\left( \frac{\partial u_2(x,y,z)}{\partial x} \right)}^2 + {\left( \frac{\partial u_3(x,y,z)}{\partial x} \right)}^2 \right]}
\end{equation}

\begin{equation}
\epsilon_{12}(x,y,z) = \frac{\partial u_1(x,y,z)}{\partial y} + \frac{\partial u_2(x,y,z)}{\partial x} + \frac{\partial u_1(x,y,z)}{\partial x} \frac{\partial u_1(x,y,z)}{\partial y} + \frac{\partial u_3(x,y,z)}{\partial x} \frac{\partial u_3(x,y,z)}{\partial y}
\end{equation}

\begin{equation}
\epsilon_{13}(x,y,z) = \frac{\partial u_1(x,y,z)}{\partial z} + \frac{\partial u_3(x,y,z)}{\partial x} + \frac{\partial u_1(x,y,z)}{\partial x} \frac{\partial u_1(x,y,z)}{\partial z} + \frac{\partial u_2(x,y,z)}{\partial x} \frac{\partial u_2(x,y,z)}{\partial z}
\end{equation}

Note that for the `large_strain` calculation, $A_y$, $A_z$ and $J$ are assumed to be zero for simplicity. This is again valid for symmetric cross-sections such as square, rectangular and circular cross-sections.

If displacement or rotational eigenstrains are provided as input, those eigenstrain increments would be subtracted from the total displacement and rotational strain increments to get the mechanical displacement and rotational strain increments.

## Stress calculation

The axial and shear strain increments, and the rotational strain increments (after removal of eigenstrains) are passed to [ComputeBeamResultants](/ComputeBeamResultants.md) to calculate the force ($\Delta F$) and moment ($\Delta M$) increments using the linear elastic constitutive relations defined in [ComputeElasticityBeam](/ComputeElasticityBeam.md) as follows:
\begin{equation}
\Delta F_1(x) = E \; \epsilon_{1}(x)
\end{equation}
\begin{equation}
\Delta F_2(x) = kG \; \epsilon_{2}(x)
\end{equation}
\begin{equation}
\Delta F_3(x) = kG \; \epsilon_{3}(x)
\end{equation}
\begin{equation}
\Delta M_1(x) = kG \; \kappa_1(x)
\end{equation}
\begin{equation}
\Delta M_2(x) = E \; \kappa_2(x)
\end{equation}
\begin{equation}
\Delta M_3(x) = E \; \kappa_3(x)
\end{equation}
where $E$ and $G$ are the Young's and shear modulus, respectively. $k$ is the shear correction factor that depends on the shape of the cross-section.

Since the strain increments are in the beam local configuration at $t+\Delta t$, the calculated force and moment increments are also in the same configuration. To get the total force and moment at $t + \Delta t$, the force and moment increments are first transformed to the global coordinate system using ${}^{t+\Delta t}R$ and then added to the forces and moments from $t$ that are already in the global coordinate system.

The strain energy is calculated as:
\begin{equation}
\delta V = \int_\Omega (\sigma_{11}(x,y,z) \delta \epsilon_{11}(x,y,z) + \sigma_{12}(x,y,z) \delta \epsilon_{12}(x,y,z) + \sigma_{13}(x,y,z) \delta \epsilon_{13}(x,y,z)) \delta \Omega
\end{equation}
\begin{equation}
= \int_{0}^{{}^0L} F_1(x) \delta \epsilon_{1}(x) + F_2(x) \delta \epsilon_{2}(x) + F_3(x) \delta \epsilon_{3}(x) + M_1(x) \delta \kappa_1(x) + M_2(x) \delta \kappa_2(x) + M_3(x) \delta \kappa_3(x)
\end{equation}

As can be seen from the equation for $\epsilon_{1}(x)$, it depends only on $\frac{\partial u_1}{\partial x}$ if $A_y$ and $A_z$ are zero. But $\epsilon_{2}(x)$ depends on both $\frac{\partial u_1}{\partial x}$ and $\theta_3$. Therefore, $F_2(x) \delta \epsilon_{2}(x)$ would contribute to the residual of both the translational DOF in y direction and rotational DOF in the z direction. Similarly, $F_3(x) \delta \epsilon_{3}(x)$ would contribute to the residual of both the translational DOF in z direction and rotational DOF in the y direction. This accounts for the coupling between the shear and bending behavior of the beam. The stress-divergence calculation is performed in [StressDivergenceBeam](/StressDivergenceBeam.md).

## Dynamic beam

There are two main ways to include the inertial effects for the beam. The first method is to assign a uniform density to the beam and calculate a consistent mass/inertia matrix for the beam, and the second method assumes that the mass of the beam is concentrated at the ends of the beam, and represents the mass of the beam using point mass/inertia at the nodes at the end of the beam.

### Consistent mass matrix

If $A_y$, $A_z$ and $Iyz$ are zero in [InertialForceBeam](/InertialForceBeam.md), then there is no coupling between the rotational and translational DOFs. The residual for the $j^{th}$ translational degree of freedom at node $i$ can be obtained as follows:

\begin{equation}
R_j^0 = \int_{0}^{{}^0L} \rho A \left(\frac{x}{{}^0L} {\ddot{u}_j}^0 + \frac{{}^0L - x}{{}^0L} {\ddot{u}_j}^1 \right) \frac{x}{{}^0L} dx
\end{equation}
\begin{equation}
R_j^1 = \int_{0}^{{}^0L} \rho A \left(\frac{x}{{}^0L} {\ddot{u}_j}^0 + \frac{{}^0L - x}{{}^0L} {\ddot{u}_j}^1 \right) \frac{{}^0L - x}{{}^0L} dx
\end{equation}
where $\rho$ is the density of the beam, which is assumed to be constant through the length of the beam, and ${\ddot{u}_j}^i$ is the translational acceleration at node $i$ in $j^{th}$ direction.

The residual for the $j^{th}$ rotational degree of freedom can be obtained as follows:
\begin{equation}
R_j^0 = \int_{0}^{{}^0L} \rho I \left(\frac{x}{{}^0L} {\ddot{\theta}_j}^0 + \frac{{}^0L - x}{{}^0L} {\ddot{\theta}_j}^1 \right) \frac{x}{{}^0L} dx
\end{equation}
\begin{equation}
R_j^1 = \int_{0}^{{}^0L} \rho I \left(\frac{x}{{}^0L} {\ddot{\theta}_j}^0 + \frac{{}^0L - x}{{}^0L} {\ddot{\theta}_j}^1 \right) \frac{{}^0L - x}{{}^0L} dx
\end{equation}
where $I = I_y + I_z$ or user provided $I_x$ for $j = 1$, $I = I_z$ for $j = 2$ and $I = I_y$ for $j = 3$.

If $A_y$ and $A_z$ are non-zero, then there is coupling between $u_1$, $\theta_2$ and $\theta_3$, $u_2$ and $\theta_1$, and $u_3$ and $\theta_1$.

### Point mass/inertia

[NodalTranslationalInertia](/NodalTranslationalInertia.md) and [NodalRotationalInertia](/NodalRotationalInertia.md) are used to apply mass/inertia at a node. The mass and inertia terms contribute only to the residual of the node at which it is applied. Mass (m) behaves isotropically in all three coordinate directions resulting in the following $j^{th}$ translational nodal residual:
\begin{equation}
R_j = m \ddot{u}_j
\end{equation}

[NodalRotationalInertia](/NodalRotationalInertia.md) takes 6 rotational inertia components as input ($I_{xx}$, $I_{yy}$, $I_{zz}$, $I_{xy}$, $I_{xz}$, $I_{yz}$). If the coordinate system in which the moment of inertia components are provided is different from the global coordinate system, then `x_orientation` and `y_orientation` need to be provided as input so that a transformation matrix from the local coordinate system to the global coordinate system can be calculated.

Once the moment of inertia tensor in the global coordinate system is obtained, the residual for the rotational DOF in the $j^{th}$ direction is:
\begin{equation}
R_j = \sum_{i=1}^3 I_{ji} \; \ddot{\theta}_i
\end{equation}

### Time-integration and damping

Rayleigh damping and Newmark and HHT time integration are calculated as in [Damping](http://mooseframework.org/wiki/PhysicsModules/TensorMechanics/Dynamics/) but with [StressDivergenceBeam](/StressDivergenceBeam.md) and [InertialForceBeam](/InertialForceBeam.md) while using consistent mass/inertia matrix, and [StressDivergenceBeam](/StressDivergenceBeam.md), [NodalTranslationalInertia](/NodalTranslationalInertia.md) and [NodalRotationalInertia](/NodalRotationalInertia.md) while using point mass/inertia matrix.

!bibtex bibliography

# Imposing Average Homogenization Constraints with the Lagrangian Kernels

## Purpose of the Homogenization System

The homogenization system is a means to impose cell-average
stress or strain conditions on periodic unit cell models
using the [new, Lagrangian kernels](LagrangianKernelTheory.md) and
specifically the [total Lagrangian](kernels/lagrangian/TotalLagrangianStressDivergence.md)
formulation.
These types of constraints are very useful for homogenization
simulations trying to determine the effective mechanical properties
of a representative volume of material.

## Mathematical Description

[!cite](danielsson2002three) describes the theory underlying the 
homogenization system.  The system aims to impose cell average 
stress or deformation constraints.  
For small deformations these constraints are of the type
\begin{equation}
      \hat{s}_{ij}=\frac{1}{V}\int_{v}s_{ij}dV
\end{equation}
or
\begin{equation}
      \hat{\varepsilon}_{ij}=\frac{1}{V}\int_{V}\varepsilon_{ij}dV
\end{equation}
and for large deformations the constraint options are:
\begin{equation}
      \hat{P}_{iJ}=\frac{1}{V}\int_{V}P_{iJ}dV
\end{equation}
or
\begin{equation}
      \hat{F}_{iJ}-\delta_{iJ}=\frac{1}{V}\int_{V}\left(F_{iJ}-\delta_{iJ}\right)dV.
\end{equation}
The "hat" quantities are targets set by the user with a MOOSE
[Function](syntax/Functions/index.md).
These targets control the cell-average stress or strain.
For large deformations the system imposes the constraints
on the 1st Piola-Kirchhoff stress or the displacement gradient.
For small deformations the system constrains the small stress or the
small strain.
As with the underlying [kernels](LagrangianKernelTheory.md), the
system uses the `large_kinematics` flag to switch between small and
large deformation theory.
Components can be mixed and matched -- the system can impose deformation
constraints in one tensor direction and stress constrains in the others.
The number of available constraints varies by the problem dimension
and the kinematic theory.  [sizes] lists the available number of constraints
and the order in which they appear when specified in the input file.
The small kinematic theory admits fewer constraints because the
relevant stress and strain tensors are symmetric.
The small deformation constraints unroll following the Voigt convention,
the large deformation constraints are the row-major unrolled full tensor

!table id=sizes caption=Number of available constraints and input file order
| Problem dimension | Small deformation constraints        | Large deformation constraints |
|-------------------|--------------------------------------|-------------------------------|
| 1                 | 1: $xx$                              | 1: $xx$                       |
| 2                 | 3: $xx$, $yy$, $xy$                  | 4: $xx$, $xy$, $yx$, $yy$     |
| 3                 | 6: $xx$, $yy$, $zz$, $yz$, $xz$ $xy$ | 9: $xx$, $xy$, $xz$, $yx$, $yy$, $yz$, $zx$, $zy$, $zz$ |

To meet these cell average stress or strain components the system introduces an extra, 
affine displacement field over the domain
\begin{equation}
      u_{i}=u_{i}^{\mu}+u_{i}^{M}
\end{equation}
where $u_{i}^\mu$ is the interpolated displacement field arising from the standard finite element
problem and 
\begin{equation}
      u_{i}^{M}=G_{iJ}X_{J}
\end{equation}
for some constant gradient tensor $G_{iJ}$.
For small deformations the gradient tensor is symmetric, for large deformations it is not.
In either case, the system adds this extra gradient to the deformation gradient
calculated from the micro displacement field $u_i^{\mu}$ before forming the strain
measures and calculating the resulting stress.
\begin{equation}
      F_{iJ}=\delta_{iJ}+u_{i,J}^{\mu}+u_{i,J}^{M}=\delta_{iJ}+u_{i,J}^{\mu}+G_{iJ}.
\end{equation}
The framework represents the constant field $G_{iJ}$ with a MOOSE [scalar variable](ScalarVariable.md).

The system implements the constrains by imposing a scalar residual equation for each specificed constraint:
\begin{equation}
      R=\int_{V}\left(X-\hat{X}\right)dV=0
\end{equation}
where $X$ represents a stress or deformation constrain, as appropriate.
This constraint is implemented as a `ScalarKernel`. 
using a [UserObject](UserObject.md) to visit each element and compute the volume integral.

## Example Simulation

[hsetup] shows a simple, but representative, sample simulation.  It consists of a square, 2D simulation cell
subdivided into four subdomain.  The material model in each subdomain is a [St. Venant-Kirchhoff](ComputeStVenantKirchhoffStress.md)
model, but the material properties in each subdomain are different as indicated on the figure.
The problem imposes a combination of deformation gradient and 1st Piola Kirchhoff constraints on the simulation cell.

!media tensor_mechanics/homogenization-setup.png
       id=hsetup
       style=width:50%;float:center;padding-top:1.5%;
       caption=Simple, 2D large deformation homogenization cell problem.

[strain] and [stress] illustrate that the cell average stress and strain match the constraints for the controlled components of the
1st Piola-Kirchhoff and deformation gradient tensor.  The uncontrolled components can vary to maintain
equilibrium in the body.  Because each subdomain has different material properties the microdisplacements are not
homogeneous, producing an inhomogeneous stress state in the simulation cell.

!media tensor_mechanics/homogenization-strain.png
       id=strain
       style=width:50%;float:center;padding-top:1.5%;
       caption=Results showing that the controlled deformation values match their targets.

!media tensor_mechanics/homogenization-stress.png
       id=stress
       style=width:50%;float:center;padding-top:1.5%;
       caption=Results showing that the controlled stress values match their targets.

The homogenization system works for 1D, 2D, and 3D problems and for any material model.

## Implementation and Requirements

The homogenization system relies on several different objects to impose the constraint residual, specify the constraints,
and add the appropriate off-diagonal Jacobian terms.
The easiest way to set up a homogenization problem is to use the [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md)
which automatically configures the correct MOOSE objects.

!alert warning
The homogenization constraint system only works with the [total Lagrangian](kernels/lagrangian/TotalLagrangianStressDivergence.md) 
implementation of the stress equilibrium kernel.  Specifically, you must use the [`HomogenizedTotalLagrangianStressDivergence`](HomogenizedTotalLagrangianStressDivergence.md)
subclass as it implements the correct off-diagonal Jacobian entries.

The basic requirements the user must setup before imposing the homogenization constraints are:

1. Setup the unconstrained problem up to the specification of boundary conditions.
2. Impose periodic boundary conditions on the simulation domain in all directions, using the [AddPeriodicBCAction](source/actions/AddPeriodicBCAction.md).
3. Add Dirichlet boundary conditions to constrain the rigid body modes.  How to do this will be problem dependent, however
   a typical method is to constrain 1 (1D), 2 (2D) or 3 (3D) individual nodes with enough Dirichlet conditions to remove the rigid
   translation and rotation modes.

These actions are *not* automatically taken care of by the action.  At this point the problem should run (i.e. be well-posed with
sufficient constraint to solve the stress equilibrium equations) but produce zero deformation or loading.

The final step the user must take is to define their constraints using MOOSE [Functions](syntax/Functions/index.md).

!alert warning
For large deformation problems in 2D and 3D the user must specify at least 1 off-diagonal displacement constraint (2D) and at 
least 3 off-diagonal displacement gradient constraints (3D).  That is, not all of the constraints can be stress constraints for
large deformations.  An unconstrained displacement gradient admits free rigid body rotation and these modes must removed.

The following steps then provide the homogenization constraints.  These steps can be replaced by 
the appropriate options in the [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md).

1. Add a `ScalarVariable` with the appropriate order for the problem being solved (see [sizes]).
2. Add a [`HomogenizationConstraint`](userobjects/lagrangian/HomogenizationConstraint.md) `UserObject` to calculate the volume-averaged constraint value.
3. Add a [`HomogenizationConstraintScalarKernel`](HomogenizationConstraintScalarKernel.md) to enforce the constraints.
4. Add a [`ComputeHomogenizedLagrangianStrain`](ComputeHomogenizedLagrangianStrain.md) material object to convert the values of the scalar variable to 
   the homogenization strain or displacement gradient.
5. Inform the [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md) object of the name of the homogenization strain with the `homogenization_gradient_names` parameter.
6. Use [`HomogenizedTotalLagrangianStressDivergence`](HomogenizedTotalLagrangianStressDivergence.md) kernels and inform the kernels of the constraints so that they can 
   include the appropriate off-diagonal Jacobian entries with the `macro_gradient`, and `constraint_types` parameters.

!bibtex bibliography

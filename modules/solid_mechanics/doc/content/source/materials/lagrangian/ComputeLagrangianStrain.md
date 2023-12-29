# ComputeLagrangianStrain

!syntax description /Materials/ComputeLagrangianStrain

## Overview

The `ComputeLagrangianStrain` class calculates the basic kinematic quantities used by
the [new mechanics kernels](LagrangianKernelTheory.md) mechanics kernels.  
[The theory](NewMaterialSystem.md) section on the new material system describes this
class's role in greater detail, the following gives a brief description.

The class calculates the strain measures available for the constitutive models to
use to define the stress update and the kinematic tenors required to setup
the stress equilibrium problem for large deformation simulations.
The `stabilize_strain` averages the volumetric/dilatational part
of these strain measures  using an [$\bar{F}$ stabilization](Stabilization.md) approach
to prevent locking of linear quad and hex elements for problems experiencing incompressible
deformation.  This class also adds in the extra deformation gradient imposed by the
[homogenization](Homogenization.md) system.

The calculations differ somewhat between small and large deformation kinematic theory.
The `large_kinematics` flag selects between the two theories.
This flag should be consistent between the strain calculator and the kernels.
The
`stabilize_strain` flag determines if the calculator uses the $\bar{F}$ stabilization [!cite](de1996design).

Step-by-step the class:

1. Calculates the deformation gradient as $F_{iJ} = \delta_{iJ} + \frac{\partial u_i}{\partial X_J}$ for large deformations or $F_{ij} = \delta_{ij} + \frac{\partial u_i}{\partial x_j}$ for small deformations.
2. If `stabilize_strain` is set, average the volumetric parts of the deformation gradient, as described in the [stabilization](Stabilization.md) documentation.
3. If active, add the extra gradient to this displacement-based deformation gradient calculated in the [ComputeLagrangianHomogenizedLagrangianStrain](ComputeLagrangianHomogenizedLagrangianStrain).
4. For large deformations only, calculate the kinematic tensors used to define the equilibrium problem in the right frame of reference in the kernels.  For small deformations these measures are set to the identity.
5. Calculate the increment in the spatial velocity gradient: $\Delta l_{ij} = \delta_{ij} - F^{(n)}_{iK} F^{(n+1)-1}_{Kj}$ for large deformations and $\Delta l_{ij} = F_{ij}^{(new)} - F_{ij}^{(old)}$ for small deformations.
6. Calculate the total strain increment as $\Delta d_{ij} = \frac{1}{2} \left( l_{ij} + l_{ji} \right)$.
7. Sum up and subtract the eigenstrain increment over the step to fine the mechanical strain increment, $\Delta \varepsilon_{ij} = \Delta d_{ij} - \Delta \varepsilon_{ij}^{(eigen)}$.
8. Use the previous step values of total and mechanical strain to calculate the updated strain values: $d_{ij}^{(new)} = d_{ij}^{(old)} + \Delta d_{ij}$ and $\varepsilon_{ij} = \varepsilon_{ij}^{(old)} + \Delta \varepsilon_{ij}$.

!alert note
The strain calculator does not apply the eigenstrains to the deformation gradient, only the incremental and integrated strain measures.

!alert tip
`stabilize_strain` should be set to true for problems using linear quad or hex elements where the material model produces incompressible or near-incompressible deformation.  It should be set to `false` for higher order elements.
The $\bar{F}$ stabilization is ineffective for linear triangle or tet elements and these elements should not be used for incompressible or near-incompressible problems.

The calculator requires `use_displaced_mesh=false` and enforces this with an error.

## Example Input File Syntax

The following example sets up the `ComputeLagrangianStrain` object for a large deformation problem without stabilization.
For small deformations the only change would be `large_kinematics=false`.

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/updated/patch/large_patch.i
         block=Materials

!syntax parameters /Materials/ComputeLagrangianStrain

!syntax inputs /Materials/ComputeLagrangianStrain

!syntax children /Materials/ComputeLagrangianStrain

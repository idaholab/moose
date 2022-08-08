# ComputeLagrangianWrappedStress

!syntax description /Materials/ComputeLagrangianWrappedStress

## Overview

This class wraps the information returned by MOOSE materials inheriting
from [StressUpdateBase](Stresses.md) for use with the
[Lagrangian kernel system](LagrangianKernelTheory.md).
To use the current MOOSE materials the user simply adds this wrapper
object to the `[Materials]` block in their input file, in addition to the
definition of the actual MOOSE material.

For small deformation problems this wrapper simply provides the engineering
strain to the MOOSE material and maps the output engineering stress and
Jacobian into the material property names expected by the Lagrangian kernels.

For large deformation problems the wrapper uses the MOOSE material to provide
the small deformation/engineering stress update, ignoring the existing
finite rotations update used in the current kernels.  Instead, this
wrapper applies the objective integration described in the
[ComputeLagraignaObjectiveStress](ComputeLagrangianObjectiveStress.md)
object to convert the engineering stress provided by the MOOSE material
to the Cauchy and Piola-Kirchhoff stresses needed by the Lagrangian kernels.
This process is adequate for most materials, but will not produce the
expected results for anisotropic materials or for materials "natively"
providing a large deformation stress update, like crystal plasticity.
For this materials users should consider writing a custom wrapper or
transitioning the material model to inherit from the new
material system [base classes](NewMaterialSystem.md).

## Example Input File Syntax

The user provides only the name of the stress and Jacobian `MaterialProperty`
produced by the MOOSE material.  For most situations the default values
are sufficient and the user doesn't have to provide any parameters for the
object.

The following example wraps a simple J2 plasticity model for use with the new
kernels.

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/updated/cross_material/correctness/plastic_j2.i
         block=Materials

!syntax parameters /Materials/ComputeLagrangianWrappedStress

!syntax inputs /Materials/ComputeLagrangianWrappedStress

!syntax children /Materials/ComputeLagrangianWrappedStress

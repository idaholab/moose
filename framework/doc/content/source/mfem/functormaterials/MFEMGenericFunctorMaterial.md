# MFEMGenericFunctorMaterial

!if! function=hasCapability('mfem')

## Overview

`MFEMGenericFunctorMaterial` defines one or more scalar material properties with values obtained from coefficients on
one or more subdomains or boundaries of the mesh, given by the [!param](/FunctorMaterials/MFEMGenericFunctorMaterial/block) or [!param](/FunctorMaterials/MFEMGenericFunctorMaterial/boundary) parameters, if provided, or applied to the
entire mesh if missing. The scalar material properties are named according to members in the
[!param](/FunctorMaterials/MFEMGenericFunctorMaterial/prop_names) parameter, with respective coefficients used to get property values given by the members of [!param](/FunctorMaterials/MFEMGenericFunctorMaterial/prop_values).

## Example Input File Syntax

!listing test/tests/mfem/kernels/gravity.i block=/FunctorMaterials remove=RigidiumWeightDensity BendiumWeightDensity

!syntax parameters /FunctorMaterials/MFEMGenericFunctorMaterial

!syntax inputs /FunctorMaterials/MFEMGenericFunctorMaterial

!syntax children /FunctorMaterials/MFEMGenericFunctorMaterial

!if-end!

!else
!include mfem/mfem_warning.md

# LevelSetBiMaterialStress
!syntax description /Materials/LevelSetBiMaterialStress

## Description
This material, `LevelSetBiMaterialStress` is intended only for use with XFEM. It determines the global stress by switching the two stresses with different base_name based on the level set values.

## Example Input File Syntax

!listing modules/xfem/test/tests/bimaterials/glued_bimaterials_2d.i block=Materials/combined

!syntax parameters /Materials/LevelSetBiMaterialStress

!syntax inputs /Materials/LevelSetBiMaterialStress

!syntax children /Materials/LevelSetBiMaterialStress

!bibtex bibliography

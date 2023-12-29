# AD Compute Smeared Cracking Stress

!syntax description /Materials/ADComputeSmearedCrackingStress

## Description
Similar to the [`ComputeSmearedCrackingStress`](/ComputeSmearedCrackingStress.md) object
except that the Jacobian of the internal forces is computed via automatic differentiation.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_smeared_cracking/cracking.i block=Materials/elastic_stress

!syntax parameters /Materials/ADComputeSmearedCrackingStress

!syntax inputs /Materials/ADComputeSmearedCrackingStress

!syntax children /Materials/ADComputeSmearedCrackingStress

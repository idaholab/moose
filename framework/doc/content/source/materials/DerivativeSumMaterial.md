# DerivativeSumMaterial

!syntax description /Materials/DerivativeSumMaterial

## Description

This material generates new material properties that sum up the values and derivatives of a specified set of function materials. Using `args` argument the union of all sets of dependent variables has to be specified so that the `DerivativeSumMaterial` can gather the necessary derivatives to sum up.

## Example usage

!listing test/tests/materials/derivative_sum_material/random_ic.i block=Materials/free_energy

!syntax parameters /Materials/DerivativeSumMaterial

!syntax inputs /Materials/DerivativeSumMaterial

!syntax children /Materials/DerivativeSumMaterial

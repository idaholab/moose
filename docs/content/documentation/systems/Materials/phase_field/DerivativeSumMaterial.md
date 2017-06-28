
# DerivativeSumMaterial
!syntax description /Materials/DerivativeSumMaterial

This material generates new material properties that sum up the values and derivatives of a specified set of function materials. Using `args` argument the union of all sets of dependent variables has to be specified so that the `DerivativeSumMaterial` can gather the necessary derivatives to sum up.

```yaml
[Materials]
  # material subblocks that define Fa, Fb, and Fc are omitted

  [./free_energy]
    type = DerivativeSumMaterial
    block = 0
    f_name = F
    sum_materials = 'Fa Fb Fc'
    args = 'c'
    outputs = exodus
  [../]
[]
```

!syntax parameters /Materials/DerivativeSumMaterial

!syntax inputs /Materials/DerivativeSumMaterial

!syntax children /Materials/DerivativeSumMaterial

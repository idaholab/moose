# ElasticEnergyMaterial

!template load file=modules/phase_field/common/ElasticEnergyMaterialDescription.md.template

```yaml
[Materials]
  # material subblocks that define stress and elasticity tensor properties
  # (and necessary derivatives) are omitted

  [elasticenergy]
    type = ElasticEnergyMaterial
    args = 'c'
  []
[]
```

!syntax parameters /Materials/ElasticEnergyMaterial

!syntax inputs /Materials/ElasticEnergyMaterial

!syntax children /Materials/ElasticEnergyMaterial

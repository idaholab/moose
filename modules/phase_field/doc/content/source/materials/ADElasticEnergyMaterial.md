# ADElasticEnergyMaterial

!template load file=modules/phase_field/common/ElasticEnergyMaterialDescription.md.template

```yaml
[Materials]
  # material subblocks that define stress and elasticity tensor properties

  [elasticenergy]
    type = ADElasticEnergyMaterial
  []
[]
```

!syntax parameters /Materials/ADElasticEnergyMaterial

!syntax inputs /Materials/ADElasticEnergyMaterial

!syntax children /Materials/ADElasticEnergyMaterial

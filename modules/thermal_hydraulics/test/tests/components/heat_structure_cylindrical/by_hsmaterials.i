# Tests that cylindrical heat structure geometry can be used.

!include part_base.i

[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 3.65
    cp = 288.734
    rho = 1.0412e2
  []
  [gap-mat]
    type = SolidMaterialProperties
    k = 1.084498
    cp = 1.0
    rho = 1.0
  []
  [clad-mat]
    type = SolidMaterialProperties
    k = 16.48672
    cp = 321.384
    rho = 6.6e1
  []
[]

[Components]
  [hs]
    materials = 'fuel-mat gap-mat clad-mat'
  []
[]

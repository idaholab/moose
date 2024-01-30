!include part_base.i

[HeatStructureMaterials]
  [hs-mat]
    type = SolidMaterialProperties
    k = 1
    cp = 1
    rho = 1
  []
[]

[Components]
  [hs]
    materials = 'hs-mat'
  []
[]

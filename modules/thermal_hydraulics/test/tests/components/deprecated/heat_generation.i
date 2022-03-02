[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 2.5
    cp = 300.
    rho = 1.032e4
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '0 -0.024748 0'
    orientation = '0 0 1'
    length = 3.865
    n_elems = 1

    names = 'fuel'
    widths = '0.004096'
    n_part_elems = '1'
    materials = 'fuel-mat'

    initial_T = 559.15
  []

  [hgen]
    type = HeatGeneration
    hs = hs
    regions = fuel
  []
[]

[Executioner]
  type = Transient
  dt = 1.e-2
[]

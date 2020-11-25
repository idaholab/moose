[HeatStructureMaterials]
  [hs_mat]
    type = SolidMaterialProperties
    rho = 1
    cp = 2
    k = 3
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    orientation = '1 0 0'
    position = '0 0 0'
    length = 1
    n_elems = 2

    names = 'blk'
    widths = '0.1'
    n_part_elems = '1'
    materials = 'hs_mat'

    initial_T = 300
  []

  [hs_boundary]
    type = HSBoundarySpecifiedTemperature
    boundary = 'hs:inner'
    hs = hs
    T = 300
  []
[]

[Executioner]
  type = Transient

  dt = 0.1
  num_steps = 1
[]

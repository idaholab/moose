[HeatStructureMaterials]
  [mat]
    type = SolidMaterialProperties
    k = 1
    cp = 1
    rho = 1
  []
[]

[Components]
  [blk]
    type = HeatStructureFromFile3D
    file = box.e
    position = '0 0 0'
    initial_T = 300
    materials = 'mat'
  []

  [left_bnd]
    type = HSBoundarySpecifiedTemperature
    hs = blk
    boundary = blk:left
    T = 300
  []

  [right_bnd]
    type = HSBoundarySpecifiedTemperature
    hs = blk
    boundary = blk:right
    T = 310
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 1
  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
[]

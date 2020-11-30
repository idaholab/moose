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
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1
    n_elems = 2
    names = 'blk'
    widths = '1'
    n_part_elems = '2'
    materials = 'hs-mat'

    initial_T = 350
  []

  [start]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:start
    T = 300
  []

  [end]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:end
    T = 400
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
[]


[Outputs]
  exodus = true
[]

[HeatStructureMaterials]
  [ss316]
    type = SolidMaterialProperties
    rho = 8.0272e3
    cp = 502.1
    k = 16.26
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    orientation = '1 0 0'
    position = '0 0 0'
    length = 1
    n_elems = 10

    inner_radius = 0.1
    widths = '0.5'
    n_part_elems = '10'
    materials = 'ss316'
    names = 'region'

    initial_T = 300
  []

  [ext_temperature]
    type = HSBoundaryExternalAppTemperature
    boundary = 'hs:outer'
    hs = hs
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 0.1
  abort_on_solve_fail = true
  solve_type = NEWTON
  line_search = basic

  nl_rel_tol = 1e-7
[]

[Outputs]
  exodus = true
  show = 'T_ext'
[]

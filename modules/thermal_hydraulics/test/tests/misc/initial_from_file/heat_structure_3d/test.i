[GlobalParams]
  initial_from_file = 'steady_state_out.e'
[]

[Materials]
  [mat]
    type = ADGenericConstantMaterial
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '16 356 6.5514e3'
  []
[]

[Functions]
  [Ts_bc]
    type = ParsedFunction
    expression = '2*sin(x*pi/2)+2*sin(pi*y) +507'
  []
[]

[Components]
  [blk]
    type = HeatStructureFromFile3D
    file = box.e
    position = '0 0 0'
  []

  [right_bnd]
    type = HSBoundarySpecifiedTemperature
    hs = blk
    boundary = blk:right
    T = Ts_bc
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100
[]

[Outputs]
  exodus = true
  execute_on = 'initial'
[]

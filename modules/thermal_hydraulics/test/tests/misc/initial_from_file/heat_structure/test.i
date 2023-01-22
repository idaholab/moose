# Test that the initial conditions read from the exodus file are correct

[GlobalParams]
  initial_from_file = 'steady_state_out.e'
[]

[HeatStructureMaterials]
  [mat1]
    type = SolidMaterialProperties
    k = 16
    cp = 356.
    rho = 6.551400E+03
  []
[]

[Functions]
  [Ts_bc]
    type = ParsedFunction
    expression = '2*sin(x*pi)+507'
  []
[]

[Components]
  [hs]
    type = HeatStructureCylindrical
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 3
    names = 'wall'
    n_part_elems = 1
    materials = 'mat1'
    widths = 0.1
  []

  [temp_outside]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:outer
    T = Ts_bc
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
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
  velocity_as_vector = false
[]

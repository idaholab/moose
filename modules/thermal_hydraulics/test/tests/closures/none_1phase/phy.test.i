# Using no closure option and setting up custom materials that computes f_D and Hw.
# In this case, these custom materials are computing just constant values

[GlobalParams]
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1 1 1e-8'

  initial_vel = 0
  initial_p = 1e5
  initial_T = 300

  closures = no_closures
[]

[FluidProperties]
  [water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Closures]
  [no_closures]
    type = Closures1PhaseNone
  []
[]

[Materials]
  [f_wall_mat]
    type = ADGenericConstantMaterial
    block = 'pipe'
    prop_names = 'f_D'
    prop_values = '0.123'
  []

  [htc_wall_mat]
    type = ADGenericConstantMaterial
    block = 'pipe'
    prop_names = 'Hw'
    prop_values = '4.321'
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    fp = water
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1e-4
    length = 1
    n_elems = 10
  []

  [inlet]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [outlet]
    type = SolidWall1Phase
    input = 'pipe:out'
  []

  [ht]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 300
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
  num_steps = 2
  dt = 1e-6
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = basic
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 5
  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  [out]
    type = Exodus
    output_material_properties = true
    show_material_properties = 'f_D Hw'
    show = 'f_D Hw'
  []
[]

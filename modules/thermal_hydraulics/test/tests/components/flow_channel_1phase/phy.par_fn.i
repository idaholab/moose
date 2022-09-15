#
# Tests the ability to set the hydraulic diameter by function.
#

D_h = 5

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e6
  initial_T = 453.1
  initial_vel = 0.0

  closures = simple_closures
[]

[Functions]
  [dh_fn]
    type = ConstantFunction
    value = ${D_h}
  []
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [left_wall]
    type = SolidWall1Phase
    input = pipe:in
  []

  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1

    A = 1.0e-4
    D_h = dh_fn

    f = 0.0

    fp = eos
  []

  [right_wall]
    type = SolidWall1Phase
    input = pipe:out
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  start_time = 0.0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100
[]

[Postprocessors]
  [D_h]
    type = ADElementIntegralMaterialProperty
    mat_prop = D_h
    block = pipe
  []
[]

[Outputs]
  csv = true
  show = 'D_h'
  execute_on = 'timestep_end'
[]

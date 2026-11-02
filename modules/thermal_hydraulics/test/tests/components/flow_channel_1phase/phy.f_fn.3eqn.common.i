# Tests that friction factor can be provided for 1-phase flow

f = 5

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 558
  initial_p = 7.0e6
  initial_vel = 0

  scaling_factor_1phase = '1e0 1e-2 1e-4'

  closures = simple_closures
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

[Functions]
  [f_func]
    type = ConstantFunction
    value = ${f}
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

  start_time = 0
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-2
  l_max_its = 30
[]

[Postprocessors]
  [f]
    type = ADElementIntegralMaterialProperty
    mat_prop = f_D
    block = pipe
  []
[]

[Outputs]
  csv = true
  show = 'f'
  execute_on = 'timestep_end'
[]

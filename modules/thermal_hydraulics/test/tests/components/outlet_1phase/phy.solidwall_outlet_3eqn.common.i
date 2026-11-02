# This test problem simulates a tube filled with steam that is suddenly opened
# on one end to an environment with a lower pressure.

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 1.43
    cv = 1040.0
    q = 2.03e6
    p_inf = 0.0
    q_prime = -2.3e4
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
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

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-5
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  start_time = 0.0
  end_time = 0.2

  dt = 0.01
  abort_on_solve_fail = true

  automatic_scaling = true
[]

[Outputs]
  file_base = 'phy.solidwall_outlet_3eqn'
  velocity_as_vector = false
  [exodus]
    type = Exodus
    show = 'p T vel'
  []

[]

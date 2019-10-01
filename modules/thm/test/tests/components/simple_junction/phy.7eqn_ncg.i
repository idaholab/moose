# Tests the simple junction component, which should enforce equality of the solution
# variables at the junction. This test ensures that the difference of the solution
# variables between both sides of the junction is very small.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T_liquid = 483
  initial_T_vapor  = 550
  initial_p_liquid = 4e6
  initial_p_vapor  = 4e6
  initial_vel_liquid = 0
  initial_vel_vapor = 0
  initial_alpha_vapor = 0.01
  initial_x_ncgs = '0.001'

  scaling_factor_2phase = '1e0
                           1e0 1e-2 1e-4
                           1e0 1e-4 1e-5'

  pressure_relaxation = true
  velocity_relaxation = true
  interface_transfer = false
  wall_mass_transfer = false

  specific_interfacial_area_max_value = 10

  closures = simple
[]

[FluidProperties]
  [./fp_air]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 28.965197e-3
  [../]
  [./fp_2p_water]
    type = StiffenedGas7EqnFluidProperties
  [../]
  [./fp]
    type = GeneralTwoPhaseNCGFluidProperties
    fp_2phase = fp_2p_water
    fp_ncgs = 'fp_air'
  [../]
[]

[Components]
  [./pipe1]
    type = FlowChannel2Phase
    fp = fp
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A   = 1.0000000000e-04
    D_h = 1.1283791671e-02
    f = 0.01
    f_interface = 0
    length = 0.25
    n_elems = 25
  [../]

  [./br1]
    type = SimpleJunction
    connections = 'pipe1:out pipe2:in'
  [../]

  [./pipe2]
    type = FlowChannel2Phase
    fp = fp
    # geometry
    position = '1 0 0'
    orientation = '1 0 0'
    A   = 1.0000000000e-04
    D_h = 1.1283791671e-02
    f = 0.01
    f_interface = 0
    length = 0.5
    n_elems = 50
  [../]

  [./inlet]
    type = InletMassFlowRateTemperature2Phase
    input = 'pipe1:in'
    m_dot_liquid = 0.084
    T_liquid = 483
    m_dot_vapor = 1.8e-5
    T_vapor = 550
    alpha_vapor = 0.01
    x_ncgs = '0.001'
  [../]

  [./outlet]
    type = Outlet2Phase
    input = 'pipe2:out'
    p_vapor = 4e6
    p_liquid = 4e6
    legacy = true
  [../]
[]

[Postprocessors]
  # beta
  [./pipe1_beta]
    type = PointValue
    variable = arhoA_liquid
    point = '0.25 0 0'
  [../]
  [./pipe2_beta]
    type = PointValue
    variable = arhoA_liquid
    point = '1 0 0'
  [../]
  [./beta_diff]
    type = DifferencePostprocessor
    value1 = pipe1_beta
    value2 = pipe2_beta
  [../]

  # arhoA_liquid
  [./pipe1_arhoA_liquid]
    type = PointValue
    variable = arhoA_liquid
    point = '0.25 0 0'
  [../]
  [./pipe2_arhoA_liquid]
    type = PointValue
    variable = arhoA_liquid
    point = '1 0 0'
  [../]
  [./arhoA_liquid_diff]
    type = DifferencePostprocessor
    value1 = pipe1_arhoA_liquid
    value2 = pipe2_arhoA_liquid
  [../]

  # arhouA_liquid
  [./pipe1_arhouA_liquid]
    type = PointValue
    variable = arhouA_liquid
    point = '0.25 0 0'
  [../]
  [./pipe2_arhouA_liquid]
    type = PointValue
    variable = arhouA_liquid
    point = '1 0 0'
  [../]
  [./arhouA_liquid_diff]
    type = DifferencePostprocessor
    value1 = pipe1_arhouA_liquid
    value2 = pipe2_arhouA_liquid
  [../]

  # arhoEA_liquid
  [./pipe1_arhoEA_liquid]
    type = PointValue
    variable = arhoEA_liquid
    point = '0.25 0 0'
  [../]
  [./pipe2_arhoEA_liquid]
    type = PointValue
    variable = arhoEA_liquid
    point = '1 0 0'
  [../]
  [./arhoEA_liquid_diff]
    type = DifferencePostprocessor
    value1 = pipe1_arhoEA_liquid
    value2 = pipe2_arhoEA_liquid
  [../]

  # arhoA_vapor
  [./pipe1_arhoA_vapor]
    type = PointValue
    variable = arhoA_vapor
    point = '0.25 0 0'
  [../]
  [./pipe2_arhoA_vapor]
    type = PointValue
    variable = arhoA_vapor
    point = '1 0 0'
  [../]
  [./arhoA_vapor_diff]
    type = DifferencePostprocessor
    value1 = pipe1_arhoA_vapor
    value2 = pipe2_arhoA_vapor
  [../]

  # arhouA_vapor
  [./pipe1_arhouA_vapor]
    type = PointValue
    variable = arhouA_vapor
    point = '0.25 0 0'
  [../]
  [./pipe2_arhouA_vapor]
    type = PointValue
    variable = arhouA_vapor
    point = '1 0 0'
  [../]
  [./arhouA_vapor_diff]
    type = DifferencePostprocessor
    value1 = pipe1_arhouA_vapor
    value2 = pipe2_arhouA_vapor
  [../]

  # arhoEA_vapor
  [./pipe1_arhoEA_vapor]
    type = PointValue
    variable = arhoEA_vapor
    point = '0.25 0 0'
  [../]
  [./pipe2_arhoEA_vapor]
    type = PointValue
    variable = arhoEA_vapor
    point = '1 0 0'
  [../]
  [./arhoEA_vapor_diff]
    type = DifferencePostprocessor
    value1 = pipe1_arhoEA_vapor
    value2 = pipe2_arhoEA_vapor
  [../]

  # arhoXA_vapor1
  [./pipe1_aXrhoA_vapor1]
    type = PointValue
    variable = aXrhoA_vapor1
    point = '0.25 0 0'
  [../]
  [./pipe2_aXrhoA_vapor1]
    type = PointValue
    variable = aXrhoA_vapor1
    point = '1 0 0'
  [../]
  [./aXrhoA_vapor1_diff]
    type = DifferencePostprocessor
    value1 = pipe1_aXrhoA_vapor1
    value2 = pipe2_aXrhoA_vapor1
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1e-4
  num_steps = 10
  abort_on_solve_fail = true
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'

  solve_type = 'PJFNK'
  nl_rel_tol = 0
  nl_abs_tol = 1e-7
  nl_max_its = 10

  l_tol = 1e-2
  l_max_its = 25

  [./Quadrature]
    type = GAUSS
    order = THIRD
  [../]
[]

[Outputs]
  [./out]
    type = CSV
    execute_postprocessors_on = 'timestep_end'
    show = 'aXrhoA_vapor1_diff beta_diff arhoA_liquid_diff arhouA_liquid_diff arhoEA_liquid_diff arhoA_vapor_diff arhouA_vapor_diff arhoEA_vapor_diff'
  [../]

  [./console]
    type = Console
    show = 'aXrhoA_vapor1_diff beta_diff arhoA_liquid_diff arhouA_liquid_diff arhoEA_liquid_diff arhoA_vapor_diff arhouA_vapor_diff arhoEA_vapor_diff'
  [../]
[]

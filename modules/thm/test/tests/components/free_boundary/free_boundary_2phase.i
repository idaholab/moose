# This test tests the FreeBoundary component in 2-phase flow.
#
# @requirement F7.2

[GlobalParams]
  initial_T_liquid = 483
  initial_T_vapor = 485
  initial_p_liquid = 1.907e6
  initial_p_vapor = 1.907e6
  initial_vel_liquid = 1
  initial_vel_vapor = 1
  initial_alpha_vapor = 0.001

  scaling_factor_2phase = '1e0 1e0 1e-2 1e-5 1e0 1e-2 1e-5'

  specific_interfacial_area_max_value = 10
  closures = simple

  gravity_vector = '0 0 0'

  f_interface = 0
[]

[FluidProperties]
  [./fp]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./inlet]
    type = FreeBoundary
    input = pipe:in
  [../]
  [./pipe]
    type = FlowChannel2Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 10
    A = 1.0

    f = 0

    fp = fp
  [../]
  [./outlet]
    type = FreeBoundary
    input = pipe:out
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  start_time = 0.0
  end_time = 0.01
  dt = 0.01
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-4
  nl_max_its = 10

  l_tol = 1e-2
  l_max_its = 20

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]

# This input file tests outflow boundary conditions for the incompressible NS equations.

[GlobalParams]
  # Dummy parameters
  gravity = '0 0 0'
  rho = 1
  mu = 1
  cp = 1
  k = 1
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.0
  ymin = 0
  ymax = 1.0
  nx = 60
  ny = 20
  elem_type = QUAD9
[]

[Variables]
  # x-velocity
  # y-velocity
  # Temperature
  # Pressure
  [./u]
    order = SECOND
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
  [./v]
    order = SECOND
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
  [./T]
    order = SECOND
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 1.0
    [../]
  [../]
  [./p]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0 # This number is arbitrary for NS...
    [../]
  [../]
[]

[Kernels]
  # mass
  # x-momentum, time
  # x-momentum, space
  # y-momentum, time
  # y-momentum, space
  # temperature
  [./mass]
    type = INSMass
    variable = p
    u = u
    v = v
    p = p
  [../]
  [./x_momentum_space]
    type = INSMomentum
    variable = u
    u = u
    v = v
    p = p
    component = 0
    integrate_p_by_parts = false
  [../]
  [./y_momentum_space]
    type = INSMomentum
    variable = v
    u = u
    v = v
    p = p
    component = 1
    integrate_p_by_parts = false
  [../]
  [./temperature_space]
    type = INSTemperature
    variable = T
    u = u
    v = v
  [../]
[]

[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = u
    boundary = 'top bottom'
    value = 0.0
  [../]
  [./inlet]
    type = DirichletBC
    variable = p
    boundary = left
    value = 1.0
  [../]
  [./y_no_slip]
    type = DirichletBC
    variable = v
    boundary = 'left top bottom'
    value = 0.0
  [../]
  [./outflow_x]
    type = INSMomentumNoBCBC
    variable = u
    u = u
    v = v
    p = p
    boundary = right
    component = 0
  [../]
  [./outflow_y]
    type = INSMomentumNoBCBC
    variable = v
    u = u
    v = v
    p = p
    boundary = right
    component = 1
  [../]
  [./outflow_T]
    type = INSTemperatureNoBCBC
    variable = T
    boundary = right
  [../]
  [./T_hot]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 1
  [../]
  [./T_cold]
    type = DirichletBC
    variable = T
    boundary = top
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    # Preconditioned JFNK (default)
    type = SMP
    full = true
    solve_type = PJFNK
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-ksp_gmres_restart '
  petsc_options_value = '300                '
  line_search = none
  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
[]

[Outputs]
  file_base = open_bc_out_pressure_BC
  exodus = true
[]


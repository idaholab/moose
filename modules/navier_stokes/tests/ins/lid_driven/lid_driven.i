[GlobalParams]
  # rho = 1000    # kg/m^3
  # mu = 0.798e-3 # Pa-s at 30C
  # cp = 4.179e3  # J/kg-K at 30C
  # k = 0.58      # W/m-K at ?C
  gravity = '0 0 0'

  # Dummy parameters
  rho = 1
  mu = 1
  cp = 1
  k = 1
[]



[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1.0
  ymin = 0
  ymax = 1.0
  nx = 16
  ny = 16
  elem_type = QUAD9
[]


[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]

  [./vel_y]
    order = SECOND
    family = LAGRANGE
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
  [../]
[]



[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    p = p
  [../]



  # x-momentum, time
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  [../]

  # x-momentum, space
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    p = p
    component = 0
  [../]



  # y-momentum, time
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  [../]

  # y-momentum, space
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    component = 1
  [../]

  [./penalty]
    type = INSCompressibilityPenalty
    variable = p
    penalty  = 1e-5
  [../]


 # temperature
 [./temperature_time]
   type = INSTemperatureTimeDerivative
   variable = T
 [../]

 [./temperature_space]
   type = INSTemperature
   variable = T
   u = vel_x
   v = vel_y
 [../]
[]




[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./lid]
    type = DirichletBC
    variable = vel_x
    boundary = 'top'
    value = 10.0
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'bottom right top left'
    value = 0.0
  [../]

 [./T_hot]
   type = DirichletBC
   variable = T
   boundary = 'bottom'
   value = 1
 [../]

 [./T_cold]
   type = DirichletBC
   variable = T
   boundary = 'top'
   value = 0
 [../]
[]



[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]


[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-2
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = 'asm      2               ilu          4'
  line_search = 'none'
  nl_rel_tol = 1e-9
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
  start_time = 0.0
  num_steps = 2
[]




[Outputs]
  file_base = lid_driven_out
  exodus = true
[]

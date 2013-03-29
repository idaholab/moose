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
  nx = 20
  ny = 20
  elem_type = QUAD9

  [./ExtraNodesets]
    [./corner_node]
      id = 99
      nodes = '0'
    [../]
  [../]
[]



[Variables]
  # x-velocity
  [./u]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

  # y-velocity
  [./v]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

 # Temperature
 [./T]
   order = SECOND
   family = LAGRANGE

   [./InitialCondition]
     type = ConstantIC
     value = 1.0
   [../]
 [../]

  # Pressure
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
  [./mass]
    type = INSMass
    variable = p
    u = u
    v = v
    p = p
  [../]



  # x-momentum, time
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = u
  [../]

  # x-momentum, space
  [./x_momentum_space]
    type = INSMomentum
    variable = u
    u = u
    v = v
    p = p
    component = 0
  [../]



  # y-momentum, time
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = v
  [../]

  # y-momentum, space
  [./y_momentum_space]
    type = INSMomentum
    variable = v
    u = u
    v = v
    p = p
    component = 1
  [../]



 # temperature
 [./temperature_time]
   type = INSTemperatureTimeDerivative
   variable = T
 [../]

 [./temperature_space]
   type = INSTemperature
   variable = T
   u = u
   v = v
   p = p
 [../]
[]




[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = u
    # boundary = '0 1 3'
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./lid]
    type = DirichletBC
    variable = u
    # boundary = '2'
    boundary = 'top'
    value = 100.0
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = v
    # boundary = '0 1 2 3'
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = '99'
    value = 0
  [../]

 [./T_hot]
   type = DirichletBC
   variable = T
   #boundary = '0'
   boundary = 'bottom'
   value = 1
 [../]

 [./T_cold]
   type = DirichletBC
   variable = T
   #boundary = '2'
   boundary = 'top'
   value = 0
 [../]
[]



[Preconditioning]
# [./FDP_Newton]
#   type = FDP
#   full = true
#   petsc_options = '-snes'
#   #petsc_options_iname = '-mat_fd_coloring_err'
#   #petsc_options_value = '1.e-10'
# [../]

[./SMP_PJFNK]
  type = SMP
  full = true
  petsc_options = '-snes_mf_operator'
[../]
[]



[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-6
  perf_log = true
  petsc_options_iname = '-ksp_gmres_restart -snes_linesearch_type'
  petsc_options_value = '300                basic'
  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 1000
  start_time = 0.0
  num_steps = 40
[]




[Output]
  file_base = lid_driven_out
  interval = 1
  output_initial = true
  exodus = true
[]

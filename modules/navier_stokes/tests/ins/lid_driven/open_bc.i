# This input file tests outflow boundary conditions for the incompressible NS equations.
[GlobalParams]
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
  xmax = 3.0
  ymin = 0
  ymax = 1.0
  nx = 60
  ny = 20
  elem_type = QUAD9
[]

[MeshModifiers]
  [./corner_node]
    type = AddExtraNodeset
    boundary = 99
    nodes = '0'
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
    boundary = 'top bottom'
    value = 0.0
  [../]

  [./inlet]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1.0
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = v
    boundary = 'left top bottom'
    value = 0.0
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = '99'
    value = 0
  [../]

 [./outflow_x]
   type = INSMomentumNoBCBC
   variable = u
   u = u
   v = v
   p = p
   boundary = 'right'
   component = 0
 [../]

 [./outflow_y]
   type = INSMomentumNoBCBC
   variable = v
   u = u
   v = v
   p = p
   boundary = 'right'
   component = 1
 [../]

 [./outflow_T]
   type = INSTemperatureNoBCBC
   variable = T
   boundary = 'right'
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  [../]
[]


[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-2
  perf_log = true
  petsc_options_iname = '-ksp_gmres_restart '
  petsc_options_value = '300                '

  line_search = 'none'

  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
  start_time = 0.0
  num_steps = 40
[]




[Outputs]
  file_base = open_bc_out
  exodus = true
[]

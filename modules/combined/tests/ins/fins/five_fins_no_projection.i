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
  type = FileMesh
  file = five_fins_second.e

  [./ExtraNodesets]
    [./corner_node]
      id = 99
      nodes = '2739'
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



[Functions]
  [./parabola]
    type = ParsedFunction
    value = 10*(-4.0)*(y-0.5)*(y+0.5)*tanh(10*t)
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
[]




[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = u
    boundary = 'top bottom fins fins_vertical'
    value = 0.0
  [../]

  [./inlet]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left'
    function = parabola
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = v
    boundary = 'left top bottom fins fins_vertical'
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
[]



[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    petsc_options = '-snes_mf_operator'
  [../]
[]


[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-2
  perf_log = true
  petsc_options_iname = '-ksp_gmres_restart -snes_linesearch_type'
  petsc_options_value = '300                basic                '
  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
  start_time = 0.0
  num_steps = 40
[]




[Output]
  file_base = five_fins_no_projection_out
  interval = 1
  output_initial = true
  exodus = true
[]

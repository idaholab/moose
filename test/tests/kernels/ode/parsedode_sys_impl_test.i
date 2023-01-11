[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Functions]
  [./f_fn]
    type = ParsedFunction
    expression = -4
  [../]
  [./bc_all_fn]
    type = ParsedFunction
    expression = x*x+y*y
  [../]

  # ODEs
  [./exact_x_fn]
    type = ParsedFunction
    expression = (-1/3)*exp(-t)+(4/3)*exp(5*t)
  [../]
[]

# NL

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]

  # ODE variables
  [./x]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
  [./y]
    family = SCALAR
    order = FIRST
    initial_condition = 2
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./uff]
    type = BodyForce
    variable = u
    function = f_fn
  [../]
[]

[ScalarKernels]
  [./td1]
    type = ODETimeDerivative
    variable = x
  [../]
  [./ode1]
    type = ParsedODEKernel
    expression = '-3*x - 2*y'
    variable = x
    coupled_variables = y
  [../]

  [./td2]
    type = ODETimeDerivative
    variable = y
  [../]
  [./ode2]
    type = ParsedODEKernel
    expression = '-4*x - y'
    variable = y
    coupled_variables = x
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = bc_all_fn
  [../]
[]

[Postprocessors]
  active = 'exact_x l2err_x'

  [./exact_x]
    type = FunctionValuePostprocessor
    function = exact_x_fn
    execute_on = 'initial timestep_end'
    point = '0 0 0'
  [../]

  [./l2err_x]
    type = ScalarL2Error
    variable = x
    function = exact_x_fn
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.01
  num_steps = 100

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = ode_sys_impl_test_out
  exodus = true
[]

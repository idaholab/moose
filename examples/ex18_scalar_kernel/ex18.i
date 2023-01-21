[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Functions]
  # ODEs
  [./exact_x_fn]
    type = ParsedFunction
    expression = (-1/3)*exp(-t)+(4/3)*exp(5*t)
  [../]
  [./exact_y_fn]
    type = ParsedFunction
    expression = (2/3)*exp(-t)+(4/3)*exp(5*t)
  [../]
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
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
    variable = diffused
  [../]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[ScalarKernels]
  [./td1]
    type = ODETimeDerivative
    variable = x
  [../]
  [./ode1]
    type = ImplicitODEx
    variable = x
    y = y
  [../]

  [./td2]
    type = ODETimeDerivative
    variable = y
  [../]
  [./ode2]
    type = ImplicitODEy
    variable = y
    x = x
  [../]
[]


[BCs]
  [./right]
    type = ScalarDirichletBC
    variable = diffused
    boundary = 1
    scalar_var = x
  [../]

  [./left]
    type = ScalarDirichletBC
    variable = diffused
    boundary = 3
    scalar_var = y
  [../]
[]

[Postprocessors]
 # to print the values of x, y into a file so we can plot it
  [./x_pp]
    type = ScalarVariable
    variable = x
    execute_on = timestep_end
  [../]

  [./y_pp]
    type = ScalarVariable
    variable = y
    execute_on = timestep_end
  [../]

  [./exact_x]
    type = FunctionValuePostprocessor
    function = exact_x_fn
    execute_on = timestep_end
    point = '0 0 0'
  [../]

  [./exact_y]
    type = FunctionValuePostprocessor
    function = exact_y_fn
    execute_on = timestep_end
    point = '0 0 0'
  [../]

  # Measure the error in ODE solution for 'x'.
  [./l2err_x]
    type = ScalarL2Error
    variable = x
    function = exact_x_fn
  [../]

  # Measure the error in ODE solution for 'y'.
  [./l2err_y]
    type = ScalarL2Error
    variable = y
    function = exact_y_fn
  [../]
[]


[Executioner]
  type = Transient
  start_time = 0
  dt = 0.01
  num_steps = 10

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

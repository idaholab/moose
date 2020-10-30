[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  parallel_type = replicated
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./test_variable_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./test_variable_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./test_variable_x_aux]
    type = FunctionDerivativeAux
    variable = test_variable_x
    component = x
    function = solution_function
  [../]
  [./test_variable_y_aux]
    type = FunctionDerivativeAux
    variable = test_variable_y
    component = y
    function = solution_function
  [../]
[]

[UserObjects]
  [./ex_soln]
    type = SolutionUserObject
    system_variables = test_variable
    mesh = solution_function_grad_p1.e
  [../]
[]

[Functions]
  [./solution_function]
    type = SolutionFunction
    solution = ex_soln
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-10
[]

[Outputs]
  file_base = solution_function_grad_p2
  exodus = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2

  nx = 5
  ny = 5

  xmin = 0.0
  xmax = 1.0

  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'bc_u bc_v f_u f_v'

  # A ParsedFunction allows us to supply analytic expressions
  # directly in the input file
  [./bc_u]
    type = ParsedFunction
    expression = sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '2'
  [../]

  [./bc_v]
    type = ParsedFunction
    expression = sin(alpha*pi*y)
    symbol_names = 'alpha'
    symbol_values = '2'
  [../]

  [./f_u]
    type = ParsedFunction
    expression = alpha*alpha*pi*pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '2'
  [../]

  [./f_v]
    type = ParsedFunction
    expression = alpha*alpha*pi*pi*sin(alpha*pi*y)
    symbol_names = 'alpha'
    symbol_values = '2'
  [../]
[]

[Kernels]
  active = 'diff_u diff_v forcing_u forcing_v'

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]

  # This Kernel can take a function name to use
  [./forcing_u]
    type = BodyForce
    variable = u
    function = f_u
  [../]

  [./forcing_v]
    type = BodyForce
    variable = v
    function = f_v
  [../]
[]

[BCs]
  active = 'all_u all_v'

  # The BC can take a function name to use
  [./all_u]
    type = FunctionDirichletBC
    variable = u
    boundary = 'bottom right top left'
    function = bc_u
  [../]

  [./all_v]
    type = FunctionDirichletBC
    variable = v
    boundary = 'bottom right top left'
    function = bc_v
  [../]
[]

[Executioner]
  type = Steady

  [./Adaptivity]
    refine_fraction = 1.0
    coarsen_fraction = 0.0
    max_h_level = 10
    steps = 3
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
    execute_on = 'initial timestep_end'
  [../]

  [./integral]
    type = ElementVectorL2Error
    var_x = u
    var_y = v
    function_x = bc_u
    function_y = bc_v
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = out
  exodus = false
  csv = true
[]

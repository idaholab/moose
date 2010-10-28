[Mesh]
  dim = 2

  [./Generation]
    nx = 10
    ny = 10
    
    x_min = 0.0
    x_max = 1.0

    y_min = 0.0
    y_max = 1.0
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'bc_func forcing_func'  
  
  # A ParsedFunction allows us to supply analytic expressions
  # directly in the input file
  [./bc_func]
    type = ParsedFunction
    value = sin(alpha*pi*x)
    vars = 'alpha'
    vals = '16'
  [../]

  # This function is an actual compiled function
  # We could have used ParsedFunction for this as well
  [./forcing_func]
    type = ExampleFunction
    alpha = 16
  [../]
[]

[Kernels]
  active = 'diff forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  # This Kernel can take a function name to use
  [./forcing]
    type = UserForcingFunction
    variable = u
    function = forcing_func
  [../]
[]

[BCs]
  active = 'all'

  # The BC can take a function name to use
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = bc_func
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true

  [./Adaptivity]
    steps = 5
    refine_fraction = 1.0
    max_h_level = 10
  [../]
[]

[Postprocessors]
  [./dofs]
    type = PrintDOFs
    variable = u
  [../]

  [./integral]
    type = ElementL2Error
    variable = u
    function = bc_func
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  postprocessor_csv = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2

  nx = 100
  ny = 100

  xmin = 0.0
  xmax = 1.0

  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [./forced]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  # A ParsedFunction allows us to supply analytic expressions
  # directly in the input file
  [./bc_func]
    type = ParsedFunction
    expression = sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '16'
  [../]

  # This function is an actual compiled function
  # We could have used ParsedFunction for this as well
  [./forcing_func]
    type = ExampleFunction
    alpha = 16
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = forced
  [../]

  # This Kernel can take a function name to use
  [./forcing]
    type = BodyForce
    variable = forced
    function = forcing_func
  [../]
[]

[BCs]
  # The BC can take a function name to use
  [./all]
    type = FunctionDirichletBC
    variable = forced
    boundary = 'bottom right top left'
    function = bc_func
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

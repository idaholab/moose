[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 32
  ny = 32
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [forced]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  # A ParsedFunction allows us to supply analytic expressions directly in the input file
  [exact]
    type = ParsedFunction
    expression = sin(alpha*pi*x)
    symbol_names = alpha
    symbol_values = 16
  []

  # This function is an actual compiled function
  [force]
    type = ExampleFunction
    alpha = 16
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = forced
  []

  # This Kernel can take a function name to use
  [forcing]
    type = ADBodyForce
    variable = forced
    function = force
  []
[]

[BCs]
  # The BC can take a function name to use
  [all]
    type = FunctionDirichletBC
    variable = forced
    boundary = 'bottom right top left'
    function = exact
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [h]
    type = AverageElementSize
  []
  [error]
    type = ElementL2Error
    variable = forced
    function = exact
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]

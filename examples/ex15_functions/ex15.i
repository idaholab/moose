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
  active = 'forced'

  [./forced]
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
    variable = forced
  [../]

  # This Kernel can take a function name to use
  [./forcing]
    type = UserForcingFunction
    variable = forced
    function = forcing_func
  [../]
[]

[BCs]
  active = 'all'

  # The BC can take a function name to use
  [./all]
    type = FunctionDirichletBC
    variable = forced
    boundary = '0 1 2 3'
    function = bc_func
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]

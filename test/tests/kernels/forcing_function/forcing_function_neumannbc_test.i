[Mesh]
  file = square.e
  uniform_refine = 4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'forcing_func exact_func'

  [./forcing_func]
    type = ParsedGradFunction
    value = 2
  [../]

  [./exact_func]
    type = ParsedFunction
    value = x*x
  [../]
[]

[Kernels]
  active = 'diff forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = UserForcingFunction
    variable = u
    function = forcing_func
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

  [./right]
    type = FunctionNeumannBC
    function = exact_func
    variable = u
    boundary = '2'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = neumannbc_out
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]

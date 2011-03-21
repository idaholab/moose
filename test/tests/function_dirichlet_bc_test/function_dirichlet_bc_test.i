[Mesh]
  dim = 2
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
  active = 'forcing_func bc_func'
  
  [./forcing_func]
    type = ParsedFunction
    value = alpha*alpha*pi*pi*sin(alpha*pi*x)
    vars = 'alpha'
    vals = '16'
  [../]

  [./bc_func]
    type = ParsedFunction
    value = sin(alpha*pi*x)
    vars = 'alpha'
    vals = '16'
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
  active = 'all'

  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 2'
    function = bc_func
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]

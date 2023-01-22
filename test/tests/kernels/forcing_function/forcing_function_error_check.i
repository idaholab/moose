[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
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
  active = 'forcing_func'

  [./forcing_func]
    type = ParsedFunction
    expression = '"alpha*alpha*pi*pi*sin(alpha*pi*x)"'
    symbol_names = 'alpha'
    symbol_values = '16'
  [../]
[]

[Kernels]
  active = 'diff forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_func
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out
  exodus = true
[]

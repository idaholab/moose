[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 5
  ny = 5
  elem_type = QUAD4
[]

[UserObjects]
  [./ud]
    type = MTUserObject
    scalar = 2
    vector = '9 7 5'
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = -2
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = x*x
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  # this kernel will user user data object from above
  [./ffn]
    type = UserObjectKernel
    variable = u
    user_object = ud
  []
[]

[BCs]
  active = 'all'

  [./all]
    type = FunctionDirichletBC
    variable = u
    function = exact_fn
    boundary = '0 1 2 3'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out
  exodus = true
[]

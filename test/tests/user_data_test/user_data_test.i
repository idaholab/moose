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
    type = MTUserData
    scalar = 2
    vector = '9 7 5'
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = -2
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = x*x
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
    type = UserDataKernel
    variable = u
    user_data = ud 
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
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  exodus = true
[]

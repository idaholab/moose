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
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = 2
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = -x*x
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
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
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

[Postprocessors]
  [./ud_pps]
    type = UserObjectPPS
    user_data = ud
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'

  restart_file_base = uo_restart_part1_out_restart_0001
[]

[Output]
  output_initial = true
  exodus = true
[]

# Solving for 2 variables, putting one into hide list and the other one into show list
# We should only see the variable that is in show list in the output.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Functions]
  [./bc_fn]
    type = ParsedFunction
    expression = x
  [../]
[]

[Variables]
  [./u]
  [../]

  [./v]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./lr_u]
    type = FunctionDirichletBC
    variable = u
    boundary = '1 3'
    function = bc_fn
  [../]

  [./lr_v]
    type = FunctionDirichletBC
    variable = v
    boundary = '1 3'
    function = bc_fn
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  console = true
  [./out]
    type = Exodus
    show = 'u'
    hide = 'v'
  [../]
[]

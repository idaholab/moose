[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 1
  ymax = 0.1
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
[]

[FVKernels]
  [./diff]
    type = FVDiffusion
    variable = u
    coeff = 0.1
  [../]
[]

[FVBCs]
  [./left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
  [./right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 10
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [./elem_left]
    type = ElementalVariableValue
    variable = u
    elementid = 0
  []
  [./elem_right]
    type = ElementalVariableValue
    variable = u
    elementid = 9
  []
[]

[Outputs]
  csv = true
[]

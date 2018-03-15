[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  [./Indicators]
    [./indicator_0]
      type = GradientJumpIndicator
      variable = u
      outputs = indicators
    [../]
    [./indicator_1]
      type = GradientJumpIndicator
      variable = u
      outputs = indicators
    [../]
  [../]
[]

[Outputs]
  [./indicators]
    type = Exodus
  [../]
  [./no_indicators]
    type = Exodus
  [../]
[]

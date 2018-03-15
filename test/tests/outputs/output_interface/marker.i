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
      outputs = none
    [../]
    [./indicator_1]
      type = GradientJumpIndicator
      variable = u
      outputs = none
    [../]
  [../]
  [./Markers]
    [./marker_0]
      type = ValueThresholdMarker
      outputs = markers
      refine = 0.5
      variable = u
    [../]
    [./marker_1]
      type = BoxMarker
      bottom_left = '0.25 0.25 0'
      top_right = '0.75 0.75 0'
      inside = REFINE
      outside = DONT_MARK
      outputs = markers
    [../]
  [../]
[]

[Outputs]
  [./markers]
    type = Exodus
  [../]
  [./no_markers]
    type = Exodus
  [../]
[]

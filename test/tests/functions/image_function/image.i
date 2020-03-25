[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Adaptivity]
  max_h_level = 5
  initial_steps = 5
  initial_marker = marker
  [Indicators]
    [indicator]
      type = GradientJumpIndicator
      variable = u
    []
  []
  [Markers]
    [marker]
      type = ErrorFractionMarker
      indicator = indicator
      refine = 0.9
    []
  []
[]

[Variables]
  [u]
  []
[]

[Functions]
  [image_func]
    type = ImageFunction
    file = stack/test_00.png
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    function = image_func
    variable = u
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

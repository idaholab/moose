[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [u]
  []
[]

[Functions]
  [image_func]
    type = ImageFunction
    file_base = stack/test
    file_suffix = png
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    threshold = 6e4
    upper_value = 1
    lower_value = -1
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

[Outputs]
  exodus = true
[]

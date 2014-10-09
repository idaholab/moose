[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 2
  ymax = 2
  uniform_refine = 1
[]

[MeshModifiers]
  [./top_left]
    type = SubdomainBoundingBox
    top_right = '1 2 0'
    bottom_left = '0 1 0'
    block_id = 1
  [../]
  [./top_right]
    type = SubdomainBoundingBox
    top_right = '2 2 0'
    bottom_left = '1 1 0'
    block_id = 1
  [../]
  [./bottom_right]
    type = SubdomainBoundingBox
    top_right = '2 1 0'
    bottom_left = '1 0 0'
    block_id = 1
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./bottom_left]
    type = ImageFunction
    file_base = stack/test
    flip_y = true
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    origin = '0 0 0'
    dimensions = '1 1 0'
  [../]
  [./top_left]
    type = ImageFunction
    file_base = stack/test
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    origin = '0 1 0'
    dimensions = '1 1 0'
    flip_x = true
  [../]
  [./top_right]
    type = ImageFunction
    origin = '1 1 0'
    file_base = stack/test
    flip_y = true
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    dimensions = '1 1 0'
  [../]
  [./bottom_right]
    type = ImageFunction
    origin = '1 0 0'
    file_base = stack/test
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    dimensions = '1 1 0'
  [../]
[]

[ICs]
  [./top_left_ic]
    function = top_left
    variable = u
    type = FunctionIC
    block = 1
  [../]
  [./bottom_left_ic]
    function = bottom_left
    variable = u
    type = FunctionIC
    block = 0
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 1
  dt = 0.1
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    nonlinear_residuals = false
    linear_residuals = false
  [../]
[]

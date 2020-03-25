[Mesh]
  uniform_refine = 1
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 40
    ymax = 2
  []
  [top]
    type = SubdomainBoundingBoxGenerator
    input = gen
    top_right = '1 2 0'
    bottom_left = '0 1 0'
    block_id = 1
  []
[]

[Variables]
  [u]
  []
[]

[Functions]
  [top]
    type = ImageFunction
    origin = '0 1 0'
    file_base = stack/test
    file_suffix = png
    flip_y = true
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    dimensions = '1 1 0'
  []
  [bottom]
    type = ImageFunction
    origin = '0 0 0'
    file_base = stack/test
    file_suffix = png
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    dimensions = '1 1 0'
  []
[]

[ICs]
  [top_ic]
    function = top
    variable = u
    type = FunctionIC
    block = 1
  []
  [bottom_ic]
    function = bottom
    variable = u
    type = FunctionIC
    block = 0
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

[Outputs]
  exodus = true
[]

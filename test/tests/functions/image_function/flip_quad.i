[Mesh]
  uniform_refine = 1
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
    xmax = 2
    ymax = 2
  []
  # Define block IDs for the four quadrants in CCW order:
  # 1=top_right
  # 2=top_left
  # 3=bottom_left
  # 4=bottom_right
  [top_right_modifier]
    input = gen
    type = SubdomainBoundingBoxGenerator
    top_right = '2 2 0'
    bottom_left = '1 1 0'
    block_id = 1
  []
  [top_left_modifier]
    input = top_right_modifier
    type = SubdomainBoundingBoxGenerator
    top_right = '1 2 0'
    bottom_left = '0 1 0'
    block_id = 2
  []
  [bottom_left_modifier]
    input = top_left_modifier
    type = SubdomainBoundingBoxGenerator
    top_right = '1 1 0'
    bottom_left = '0 0 0'
    block_id = 3
  []
  [bottom_right_modifier]
    input = bottom_left_modifier
    type = SubdomainBoundingBoxGenerator
    top_right = '2 1 0'
    bottom_left = '1 0 0'
    block_id = 4
  []
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [bottom_left_func]
    type = ImageFunction
    file_base = stack/test
    flip_x = true
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    file_suffix = png
    origin = '0 0 0'
    dimensions = '1 1 0'
  []
  [top_left_func]
    type = ImageFunction
    file_base = stack/test
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    file_suffix = png
    origin = '0 1 0'
    dimensions = '1 1 0'
    flip_x = true
    flip_y = true
  []
  [top_right_func]
    type = ImageFunction
    origin = '1 1 0'
    file_base = stack/test
    file_suffix = png
    flip_y = true
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    dimensions = '1 1 0'
  []
  [bottom_right_func]
    type = ImageFunction
    origin = '1 0 0'
    file_base = stack/test
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    file_suffix = png
    dimensions = '1 1 0'
  []
[]

[ICs]
  # Defined the same way as the MeshGenerators
  [top_right_ic]
    function = top_right_func
    variable = u
    type = FunctionIC
    block = 1
  []
  [top_left_ic]
    function = top_left_func
    variable = u
    type = FunctionIC
    block = 2
  []
  [bottom_left_ic]
    function = bottom_left_func
    variable = u
    type = FunctionIC
    block = 3
  []
  [bottom_right_ic]
    function = bottom_right_func
    variable = u
    type = FunctionIC
    block = 4
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

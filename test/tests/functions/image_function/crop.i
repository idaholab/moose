[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  uniform_refine = 2
  xmin = 0.5
  ymin = 0.5
[]

[Variables]
  [u]
  []
[]

[Functions]
  [image_func]
    type = ImageFunction
    file_base = stack/test
    file_range = '0' # file_range is a vector input, a single entry means "read only 1 file"
    file_suffix = png
    origin = '0 0 0'
    dimensions = '1 1 0'
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

[Outputs]
  exodus = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
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

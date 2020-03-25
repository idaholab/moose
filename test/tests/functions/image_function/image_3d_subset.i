[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 20
  ny = 20
  nz = 6
  zmax = 0.3
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
    file_range = '0 5'
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

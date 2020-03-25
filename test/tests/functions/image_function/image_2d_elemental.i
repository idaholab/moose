[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [image_func]
    type = ImageFunction
    file_base = stack/test
    file_suffix = png
    # file range is parsed as a vector of unsigned.  If it only has 1
    # entry, only a single file is read.
    file_range = '0'
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
  type = Steady
[]

[Outputs]
  exodus = true
[]

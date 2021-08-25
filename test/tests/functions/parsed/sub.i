[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[AuxVariables]
  [f]
  []
  [f_offset]
  []
[]

[Functions]
  [f_offset]
    type = ParsedFunction
    value = 'x+y+z'
    use_app_position_offset = true
  []
  [f]
    type = ParsedFunction
    value = 'x+y+z'
  []
[]

[AuxKernels]
  [f]
    type = FunctionAux
    variable = f
    function = f
  []
  [f_offset]
    type = FunctionAux
    variable = f_offset
    function = f_offset
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

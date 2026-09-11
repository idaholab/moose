[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
[]

[AuxVariables]
  [disp_x][]
  [disp_y][]
  [disp_z][]
[]

[AuxKernels]
  [disp_x]
    type = FunctionAux
    function = '1'
    variable = disp_x
  []
  [disp_y]
    type = FunctionAux
    function = '0.1*y'
    variable = disp_y
  []
  [disp_z]
    type = FunctionAux
    function = '1+0.1*z'
    variable = disp_z
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

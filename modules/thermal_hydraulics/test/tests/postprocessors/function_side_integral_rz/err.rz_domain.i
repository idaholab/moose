[Mesh]
  [mg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  coord_type = RSPHERICAL
[]

[Postprocessors]
  [el_int]
    type = FunctionSideIntegralRZ
    boundary = right
    axis_point = '0 0 0'
    axis_dir = '0 1 0'
    function = 1
    execute_on = 'initial'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

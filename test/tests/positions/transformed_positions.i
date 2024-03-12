[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Positions]
  [input]
    type = InputPositions
    positions = '0 0 0
                 1 0 0
                 0 1 0'
    outputs = none
  []
  [scale]
    type = TransformedPositions
    base_positions = 'input'
    vector_value = '1 2 3'
    transform = 'SCALE'
  []
  [rotate]
    type = TransformedPositions
    base_positions = 'input'
    vector_value = '0 0 90'
    transform = 'ROTATE_XYZ'
  []
  [translate]
    type = TransformedPositions
    base_positions = 'input'
    vector_value = '10 0 0'
    transform = 'TRANSLATE'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]

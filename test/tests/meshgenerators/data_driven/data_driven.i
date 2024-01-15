[Mesh]
  [nx]
    type = TestDataDrivenGenerator
    nx = 4
  []
  [ny]
    type = TestDataDrivenGenerator
    ny = 2
  []
  [mesh]
    type = TestDataDrivenGenerator
    nx_generator = nx
    ny_generator = ny
  []
  [transform]
    type = TransformGenerator
    input = mesh
    transform = translate
    vector_value = '10 5 0'
  []
  data_driven_generator = mesh
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

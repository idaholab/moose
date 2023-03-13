[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Materials]
  [./tensor]
    type = ADGenericConstantRankTwoTensor
    tensor_name = constant
    # tensor values are column major-ordered
    tensor_values = '1 4 7 2 5 8 3 6 9'
    outputs = all
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [00]
    type = ElementAverageValue
    variable = constant_00
  []
  [01]
    type = ElementAverageValue
    variable = constant_01
  []
  [02]
    type = ElementAverageValue
    variable = constant_02
  []
  [10]
    type = ElementAverageValue
    variable = constant_10
  []
  [11]
    type = ElementAverageValue
    variable = constant_11
  []
  [12]
    type = ElementAverageValue
    variable = constant_12
  []
  [20]
    type = ElementAverageValue
    variable = constant_20
  []
  [21]
    type = ElementAverageValue
    variable = constant_21
  []
  [22]
    type = ElementAverageValue
    variable = constant_22
  []
[]

[Outputs]
  csv = true
[]

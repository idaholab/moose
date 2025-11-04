[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Materials]
  [tensor]
    type = ADGenericConstantSymmetricRankTwoTensor
    tensor_name = constant
    tensor_values = '1 2 3 4 5 6' # Note mandel factor kicks in for the off-diagonal entries
    outputs = all
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [0]
    type = ElementAverageValue
    variable = constant_0
  []
  [1]
    type = ElementAverageValue
    variable = constant_1
  []
  [2]
    type = ElementAverageValue
    variable = constant_2
  []
  [3]
    type = ElementAverageValue
    variable = constant_3
  []
  [4]
    type = ElementAverageValue
    variable = constant_4
  []
  [5]
    type = ElementAverageValue
    variable = constant_5
  []
[]

[Outputs]
  csv = true
[]

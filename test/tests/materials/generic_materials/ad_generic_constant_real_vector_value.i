[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Materials]
  [vector]
    type = ADGenericConstantRealVectorValue
    vector_name = constant
    vector_values = '1 2 3'
    outputs = all
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]

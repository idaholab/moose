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
    type = GenericConstantVectorMaterial
    prop_names = 'constant_3 constant_2'
    prop_values = '1 2 3; 0 1'
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

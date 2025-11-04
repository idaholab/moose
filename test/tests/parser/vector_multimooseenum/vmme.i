[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Materials]
  [vmme]
    type = VecMultiMooseEnumMaterial
    mme = 'b; c a; a b c d e; e'
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

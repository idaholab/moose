[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./coupled]
    type = CoupledForce
    variable = u
    # 'a' does not exist -> error
    v = a
  [../]
[]

[Executioner]
  type = Steady
[]

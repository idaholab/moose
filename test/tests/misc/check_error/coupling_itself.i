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
    v = u
  [../]
[]

[Executioner]
  type = Steady
[]

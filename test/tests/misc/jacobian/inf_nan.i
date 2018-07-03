[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./td]
    type = NanKernel
    variable = u
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

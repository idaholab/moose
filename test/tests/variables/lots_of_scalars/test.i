[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[LotsOfVariables]
  [./scalars]
    number = 1000
    family = SCALAR
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[Postprocessors]
  [./ndofs]
    type = NumDOFs
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = PPBindingSteady
  postprocessor = ndofs
[]

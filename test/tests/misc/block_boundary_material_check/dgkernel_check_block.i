[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[DGKernels]
  [./dg]
    type = MatDGKernel
    mat_prop = 'foo'
    variable = u
    block = 0
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

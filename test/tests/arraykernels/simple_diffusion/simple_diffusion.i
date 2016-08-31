[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 20
[]

[Variables]
  [./u]
    count = 2
  [../]
[]

[ArrayKernels]
  [./diff]
    type = ArrayDiffusion
    variable = u
  [../]
[]

[ArrayBCs]
  [./left]
    type = ArrayDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = ArrayDirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Problem]
  type = FEProblem
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'JFNK'
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]

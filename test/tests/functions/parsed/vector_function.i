[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  uniform_refine = 1
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./conductivity]
    type = ParsedVectorFunction
    value_y = 0.1
    value_x = 0.8
  [../]
[]

[Kernels]
  [./diff]
    type = DiffTensorKernel
    variable = u
    conductivity = conductivity
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]

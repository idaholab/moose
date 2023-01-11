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
    expression_y = 0.1
    expression_x = 0.8
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
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

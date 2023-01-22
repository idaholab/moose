# use the vectorValue of a LinearCombinationFunction
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
  [./conductivity_1]
    type = ParsedVectorFunction
    expression_y = '0.1+x'
    expression_x = '0.5*(1+x*y)'
  [../]
  [./conductivity_2]
    type = ParsedVectorFunction
    expression_y = '0.1+2*x'
    expression_x = '0.2+x*y'
  [../]
  [./conductivity]
    type = LinearCombinationFunction # yields value_y=0.1, value_x=0.8
    functions = 'conductivity_1 conductivity_2'
    w = '2 -1'
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

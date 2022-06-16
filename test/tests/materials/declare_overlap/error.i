[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 1
  []
  [./left_domain]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
    block_id = 10
  [../]
[]


[Variables]
  [./u]
    initial_condition = 2
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = 'p'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 2
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 3
  [../]
[]

[Materials]

  [./all]
    type = GenericConstantMaterial
    prop_names =  'f f_prime p'
    prop_values = '2 2.5     2.468'
    block = ANY_BLOCK_ID
    outputs = all
  [../]

  [./left]
    type = GenericConstantMaterial
    prop_names =  'f f_prime p'
    prop_values = '1 0.5     1.2345'
    block = 10
    outputs = all
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
[]

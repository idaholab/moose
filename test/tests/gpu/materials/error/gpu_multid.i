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

[GPUKernels]
  [./diff]
    type = GPUDiffusion
    variable = u
  [../]
[]

[GPUBCs]
  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = left
    value = 2
  [../]
  [./right]
    type = GPUDirichletBC
    variable = u
    boundary = right
    value = 3
  [../]
[]

[GPUMaterials]
  [left_real_1D]
    type = GPU1DRealProperty
    block = 0
    name = 'prop_a'
    dims = '1'
  []
  [right_real_1D]
    type = GPU1DRealProperty
    block = 10
    name = 'prop_a'
    dims = '2'
  []
  [right_real_1D_correct]
    type = GPU1DRealProperty
    block = 10
    name = 'prop_a'
    dims = '1'
  []
  [right_real_2D]
    type = GPU2DRealProperty
    block = 10
    name = 'prop_a'
    dims = '1 1'
  []
  [right_int_1D]
    type = GPU1DIntProperty
    block = 10
    name = 'prop_a'
    dims = '1'
  []
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

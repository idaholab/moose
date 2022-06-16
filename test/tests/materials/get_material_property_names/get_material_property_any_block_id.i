[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [./add_subdomain]
    input = gen
    type = SubdomainBoundingBoxGenerator
    top_right = '1 1 0'
    bottom_left = '0 0.5 0'
    block_id = 100
    block_name = 'top'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./block]
    type = GenericConstantMaterial
    prop_names = block_prop
    block = ANY_BLOCK_ID
    prop_values = 12345
  [../]
[]

[UserObjects]
  [./get_material_block_names_test]
    type = GetMaterialPropertyBoundaryBlockNamesTest
    expected_names = 'ANY_BLOCK_ID'
    property_name = 'block_prop'
    test_type = 'block'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

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
  [./boundary]
    type = GenericConstantMaterial
    prop_names = boundary_prop
    boundary = ANY_BOUNDARY_ID
    prop_values = 54321
  [../]
[]

[UserObjects]
  [./get_material_boundary_names_test]
    type = GetMaterialPropertyBoundaryBlockNamesTest
    expected_names = 'ANY_BOUNDARY_ID'
    property_name = 'boundary_prop'
    test_type = 'boundary'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

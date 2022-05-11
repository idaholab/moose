[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
  [./material]
    type = GenericConstantMaterial
    prop_names = combo
    boundary = 'left right'
    prop_values = 12345
  [../]
[]

[UserObjects]
  [./get_material_boundary_names_test]
    type = GetMaterialPropertyBoundaryBlockNamesTest
    expected_names = 'left right'
    property_name = combo
    test_type = 'boundary'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

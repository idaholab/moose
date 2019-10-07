# Tests the Jacobians of the OneDMomentumGravity kernel

[JacobianTestGeneral]
  variable_names = 'beta arhoA arhouA'
  variable_values = '2 3 4'
  aux_variable_names = 'A'
  aux_variable_values = '5'
  snes_test_err = 1e-8
  generate_mesh = false
[]

[Mesh]
  file = ../meshes/skew_1elem.e
  construct_side_list_from_node_list = true
[]

[Materials]
  [./alpha_material]
    type = LinearTestMaterial
    name = alpha
    vars = 'beta'
    slopes = '6'
  [../]
  [./rho_material]
    type = LinearTestMaterial
    name = rho
    vars = 'beta arhoA'
    slopes = '1.5 2.5'
  [../]
  [./dir_material]
    type = DirectionMaterial
  [../]
[]

[Kernels]
  [./test]
    type = OneDMomentumGravity
    variable = arhouA
    A = A
    beta = beta
    arhoA = arhoA
    direction = direction
    alpha = alpha
    rho = rho
    gravity_vector = '1 2 3'
  [../]
[]

# Tests the Jacobians of the OneD3EqnEnergyGravity kernel

[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
  variable_values = '3 4 5'
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
  [rho_material]
    type = LinearTestMaterial
    name = rho
    vars = 'rhoA'
    slopes = '2.5'
  []
  [vel_material]
    type = LinearTestMaterial
    name = vel
    vars = 'rhoA rhouA'
    slopes = '3.5 4.5'
  []
  [dir_material]
    type = DirectionMaterial
  []
[]

[Kernels]
  [test]
    type = OneD3EqnEnergyGravity
    variable = rhoEA
    A = A
    arhoA = rhoA
    arhouA = rhouA
    direction = direction
    rho = rho
    vel = vel
    gravity_vector = '1 2 3'
  []
[]

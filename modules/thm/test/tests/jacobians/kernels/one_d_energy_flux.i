# Tests the Jacobians of OneDEnergyFlux for single-phase flow

[JacobianTestGeneral]
  variable_names = 'arhoA arhouA arhoEA'
  variable_values = '2.0 3.0 4.0'
  aux_variable_names = 'A'
  aux_variable_values = '5.0'
  snes_test_err = 1e-8
  generate_mesh = false
[]

[Mesh]
  file = ../meshes/skew_1elem.e
  construct_side_list_from_node_list = true
[]

[Materials]
  [./rho_material]
    type = LinearTestMaterial
    name = rho
    vars = 'arhoA'
    slopes = '1.2'
  [../]
  [./vel_material]
    type = LinearTestMaterial
    name = vel
    vars = 'arhoA arhouA'
    slopes = '1.4 2.4'
  [../]
  [./e_material]
    type = LinearTestMaterial
    name = e
    vars = 'arhoA arhouA arhoEA'
    slopes = '1.6 2.6 3.6'
  [../]
  [./p_material]
    type = LinearTestMaterial
    name = p
    vars = 'arhoA arhouA arhoEA'
    slopes = '1.8 2.8 3.8'
  [../]
  [./dir_material]
    type = DirectionMaterial
  [../]
[]

[Kernels]
  [./energy_flux]
    type = OneDEnergyFlux
    variable = arhoEA
    A = A
    arhoA = arhoA
    arhouA = arhouA
    arhoEA = arhoEA
    direction = direction
    alpha = unity
    rho = rho
    vel = vel
    e = e
    p = p
  [../]
[]

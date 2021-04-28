# Tests the Jacobians of OneD3EqnEnergyFlux for single-phase flow

[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
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
  [rho_material]
    type = LinearTestMaterial
    name = rho
    vars = 'rhoA'
    slopes = '1.2'
  []
  [vel_material]
    type = LinearTestMaterial
    name = vel
    vars = 'rhoA rhouA'
    slopes = '1.4 2.4'
  []
  [e_material]
    type = LinearTestMaterial
    name = e
    vars = 'rhoA rhouA rhoEA'
    slopes = '1.6 2.6 3.6'
  []
  [p_material]
    type = LinearTestMaterial
    name = p
    vars = 'rhoA rhouA rhoEA'
    slopes = '1.8 2.8 3.8'
  []
  [dir_material]
    type = DirectionMaterial
  []
[]

[Kernels]
  [energy_flux]
    type = OneD3EqnEnergyFlux
    variable = rhoEA
    A = A
    arhoA = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
    direction = direction
    rho = rho
    vel = vel
    e = e
    p = p
  []
[]

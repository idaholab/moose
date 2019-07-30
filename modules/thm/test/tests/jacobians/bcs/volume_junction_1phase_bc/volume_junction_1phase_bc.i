[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
  variable_values = '1.8 0.3 3.2'
  scalar_variable_names = 'rhoV rhouV rhovV rhowV rhoEV'
  scalar_variable_values = '1.2 0.2 0.5 0.8 2.9'
  aux_variable_names = 'A_elem A_linear'
  aux_variable_values = '1.1 1.2'
  snes_test_err = 1e-6
  generate_mesh = false
[]

[Mesh]
  file = ../../meshes/skew_2channel_1elem.e
  construct_side_list_from_node_list = true
[]

[BCs]
  [./bc1]
    type = VolumeJunction1PhaseBC
    variable = rhoA
    boundary = 1
    normal = -1
    connection_index = 0
    volume_junction_uo = volume_junction_uo
    A_elem = A_elem
    A_linear = A_linear
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    rhoV = rhoV
    rhouV = rhouV
    rhovV = rhovV
    rhowV = rhowV
    rhoEV = rhoEV
  [../]
  [./bc2]
    type = VolumeJunction1PhaseBC
    variable = rhoA
    boundary = 3
    normal = -1
    connection_index = 1
    volume_junction_uo = volume_junction_uo
    A_elem = A_elem
    A_linear = A_linear
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    rhoV = rhoV
    rhouV = rhouV
    rhovV = rhovV
    rhowV = rhowV
    rhoEV = rhoEV
  [../]
[]

[Materials]
  [./direction_mat]
    type = DirectionMaterial
  [../]
[]

[UserObjects]
  [./numerical_flux1]
    type = NumericalFlux3EqnCentered
    fluid_properties = fp
    execute_on = 'initial linear nonlinear'
  [../]
  [./numerical_flux2]
    type = NumericalFlux3EqnCentered
    fluid_properties = fp
    execute_on = 'initial linear nonlinear'
  [../]
  [./volume_junction_uo]
    type = VolumeJunction1PhaseUserObject
    volume = 0.3
    A = A_elem
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    rhoV = rhoV
    rhouV = rhouV
    rhovV = rhovV
    rhowV = rhowV
    rhoEV = rhoEV
    fp = fp
    boundary = '1 3'
    normals = '-1 -1'
    numerical_flux_names = 'numerical_flux1 numerical_flux2'
    execute_on = 'initial linear nonlinear'
  [../]
[]

[FluidProperties]
  [./fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 28.67055e-3
  [../]
[]

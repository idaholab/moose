[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
  variable_values = '1.8 0.3 3.2'
  scalar_variable_names = 'rhoV rhouV rhovV rhowV rhoEV'
  scalar_variable_values = '1.2 0.8 0.8 0.8 2.9'
  aux_variable_names = 'A_elem A_linear'
  aux_variable_values = '1.1 1.2'
  snes_test_err = 1e-6
  generate_mesh = false
[]

[Mesh]
  file = ../../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[ScalarKernels]
  [scalar_kernel]
    type = VolumeJunctionAdvectionScalarKernel
    variable = rhoV
    equation_index = 0
    volume_junction_uo = pump_uo
  []
[]

[Materials]
  [direction_mat]
    type = DirectionMaterial
  []
[]

[UserObjects]
  [numerical_flux1]
    type = NumericalFlux3EqnCentered
    fluid_properties = fp
    execute_on = 'initial linear nonlinear'
  []
  [numerical_flux2]
    type = NumericalFlux3EqnCentered
    fluid_properties = fp
    execute_on = 'initial linear nonlinear'
  []
  [pump_uo]
    type = Pump1PhaseUserObject
    volume = 0.3
    A_ref = 2.34
    K = 0
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
    boundary = '2 3'
    normals = '1 -1'
    numerical_flux_names = 'numerical_flux1 numerical_flux2'
    execute_on = 'initial linear nonlinear'
    head = 95
    gravity_magnitude = 9.81
  []
[]

[Modules/FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 28.67055e-3
  []
[]

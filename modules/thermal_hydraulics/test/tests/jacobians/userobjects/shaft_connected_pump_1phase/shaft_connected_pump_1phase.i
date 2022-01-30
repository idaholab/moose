# Pump data used in this test comes from the LOFT Systems Tests, described in NUREG/CR-0247

[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
  variable_values = '2.0 3.0 4.0'
  scalar_variable_names = 'rhoV rhouV rhovV rhowV rhoEV omega'
  scalar_variable_values = '1.2 0.2 0.5 0.8 2.9 314'
  aux_variable_names = 'A_elem A_linear'
  aux_variable_values = '1.1 1.2'
  use_transient_executioner = true
  snes_test_err = 1e-9
  generate_mesh = false
[]

[Mesh]
  file = ../../meshes/skew_2channel_1elem.e
  construct_side_list_from_node_list = true
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
    type = ShaftConnectedPump1PhaseUserObject
    fp = fp
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
    omega = omega
    boundary = '1 3'
    normals = '-1 -1'
    A_ref = 1.23
    K = 0
    gravity_magnitude = 9.81
    di_out = '1 2 3'
    numerical_flux_names = 'numerical_flux1 numerical_flux2'
    execute_on = 'initial linear nonlinear'
    inertia_coeff = '1 1 1 1'
    inertia_const = 1.61397
    omega_rated = 314
    speed_cr_I = 1e12
    speed_cr_fr = 0
    torque_rated = 47.1825
    volumetric_rated = 1.3
    head_rated = 58.52
    tau_fr_coeff = '0 2 9.084 0'
    tau_fr_const = 1
    head = head_fcn
    torque_hydraulic = torque_fcn
    inlet = inlet
    outlet = outlet
    pump_name = pump
    density_rated = 4
  []
[]

[Modules/FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
[]

[Functions]
  [head_fcn]
    type = PiecewiseLinear
    x = '0 3.14 6.28'
    y = '-0.66 1 -0.66'
  []
  [torque_fcn]
    type = PiecewiseLinear
    x = '0 3.14 6.28'
    y = '-0.66 1.3 -0.66'
  []
[]

[ScalarKernels]
  [shaft_td]
    type = ShaftTimeDerivativeScalarKernel
    variable = omega
    uo_names = pump_uo
  []
  [shaft_total_torque]
    type = ShaftComponentTorqueScalarKernel
    variable = omega
    shaft_connected_component_uo = pump_uo
  []

  [jct_ask]
    type = VolumeJunctionAdvectionScalarKernel
    variable = rhoV
    volume_junction_uo = pump_uo
    equation_index = 0
  []
[]

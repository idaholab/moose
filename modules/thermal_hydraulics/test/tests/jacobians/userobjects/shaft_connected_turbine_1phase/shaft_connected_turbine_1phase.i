# Turbine data used in this test comes from the LOFT Systems Tests, described in NUREG/CR-0247

[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
  variable_values = '2.0 30 4e5'
  scalar_variable_names = 'rhoV rhouV rhovV rhowV rhoEV omega'
  scalar_variable_values = '1.2 3e2 2e2 1e2 3e5 314'
  aux_variable_names = 'A_elem A_linear'
  aux_variable_values = '1.1 1.2'
  use_transient_executioner = true
  snes_test_err = 1e-10
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
  [turbine_uo]
    type = ShaftConnectedTurbine1PhaseUserObject
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
    di_out = '1 2 3'
    numerical_flux_names = 'numerical_flux1 numerical_flux2'
    execute_on = 'initial linear nonlinear'
    inertia_coeff = '1 1 1 1'
    inertia_const = 1.61397
    omega_rated = 314
    D_wheel = 0.4
    speed_cr_I = 1e12
    speed_cr_fr = 0
    tau_fr_coeff = '0 2 9.084 0'
    tau_fr_const = 1
    inlet = inlet
    outlet = outlet
    turbine_name = turbine
    head_coefficient = head
    power_coefficient = power
  []
[]

[Modules/FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
[]

[Functions]
  [head]
    type = PiecewiseLinear
    x = '0 0.1 1'
    y = '0 15 20'
  []
  [power]
    type = PiecewiseLinear
    x = '0 0.1 1'
    y = '0 0.05 0.18'
  []
[]

[ScalarKernels]
  [shaft_td]
    type = ShaftTimeDerivativeScalarKernel
    variable = omega
    uo_names = turbine_uo
  []
  [shaft_total_torque]
    type = ShaftComponentTorqueScalarKernel
    variable = omega
    shaft_connected_component_uo = turbine_uo
  []

  [jct_ask]
    type = VolumeJunctionAdvectionScalarKernel
    variable = rhoV
    volume_junction_uo = turbine_uo
    equation_index = 0
  []
[]

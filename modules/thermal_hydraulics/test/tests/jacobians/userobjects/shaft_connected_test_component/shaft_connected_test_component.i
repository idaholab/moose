[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
  variable_values = '2.0 3.0 4.0'
  scalar_variable_names = 'jct_var omega'
  scalar_variable_values = '2 10'
  aux_variable_names = 'A_elem A_linear'
  aux_variable_values = '1.1 1.2'
  use_transient_executioner = true
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
  [sc_test_comp_uo]
    type = ShaftConnectedTestComponentUserObject
    volume = 0.3
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    jct_var = jct_var
    omega = omega
    boundary = '1 3'
    normals = '-1 -1'
    # note the actual names in 'numerical_flux_names' are not used for anything
    numerical_flux_names = 'numerical_flux1 numerical_flux2'
    execute_on = 'initial linear nonlinear'
  []
[]

[ScalarKernels]
  [shaft_td]
    type = ShaftTimeDerivativeScalarKernel
    variable = omega
    uo_names = sc_test_comp_uo
  []
  [shaft_total_torque]
    type = ShaftComponentTorqueScalarKernel
    variable = omega
    shaft_connected_component_uo = sc_test_comp_uo
  []

  [jct_td]
    type = ODETimeDerivative
    variable = jct_var
  []
  [jct_ask]
    type = VolumeJunctionAdvectionScalarKernel
    variable = jct_var
    volume_junction_uo = sc_test_comp_uo
    equation_index = 0
  []
[]

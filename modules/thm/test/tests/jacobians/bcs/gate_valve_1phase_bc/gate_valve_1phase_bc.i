[JacobianTest1PhaseRDG]
  ic_option = riemann_LM
  A_function = A_fn
  snes_test_err = 1e-6
  generate_mesh = false
[]

[Mesh]
  file = ../../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[Functions]
  [./A_fn]
    type = PiecewiseFunction
    axis = x
    axis_coordinates = '1.05'
    functions = '2.0 2.5'
  [../]
[]

[BCs]
  [./bc1]
    type = GateValve1PhaseBC
    variable = rhoA
    boundary = 1
    normal = -1
    connection_index = 0
    gate_valve_uo = junction_uo
    A_elem = A
    A_linear = A_linear
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
  [../]
  [./bc2]
    type = GateValve1PhaseBC
    variable = rhoA
    boundary = 3
    normal = -1
    connection_index = 1
    gate_valve_uo = junction_uo
    A_elem = A
    A_linear = A_linear
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
  [../]
[]

[UserObjects]
  [./numerical_flux]
    type = NumericalFlux3EqnCentered
    fluid_properties = fluid_properties
    execute_on = 'initial linear nonlinear'
  [../]
  [./junction_uo]
    type = GateValve1PhaseUserObject
    open_area_fraction = 0.5
    A = A
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    boundary = '1 3'
    normals = '-1 -1'
    numerical_flux = numerical_flux
    component_name = valve
    execute_on = 'initial linear nonlinear'
  [../]
[]

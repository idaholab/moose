[JacobianTest1PhaseRDG]
  ic_option = riemann_LM
  snes_test_err = 1e-6
  generate_mesh = false
[]

[Mesh]
  file = ../../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[BCs]
  [./bc1]
    type = JunctionOneToOne1PhaseBC
    variable = rhoA
    boundary = 1
    normal = -1
    connection_index = 0
    junction_uo = junction_uo
    A_elem = A
    A_linear = A_linear
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
  [../]
  [./bc2]
    type = JunctionOneToOne1PhaseBC
    variable = rhoA
    boundary = 3
    normal = -1
    connection_index = 1
    junction_uo = junction_uo
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
    type = JunctionOneToOne1PhaseUserObject
    A = A
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    boundary = '1 3'
    normals = '-1 -1'
    numerical_flux = numerical_flux
    junction_name = junction
    execute_on = 'initial linear nonlinear'
  [../]
[]

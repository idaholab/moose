# Tests the Jacobians of the OneD3EqnEnergyHeatSource kernel

[JacobianTestGeneral]
  variable_names = 'rhoEA'
  variable_values = '5'
  aux_variable_names = 'A'
  aux_variable_values = '5'
  snes_test_err = 1e-8
  generate_mesh = false
[]

[Mesh]
  file = ../meshes/skew_1elem.e
  construct_side_list_from_node_list = true
[]

[Kernels]
  [test]
    type = OneD3EqnEnergyHeatSource
    variable = rhoEA
    A = A
    q = 1
  []
[]

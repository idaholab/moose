[JacobianTest1Phase]
  A = 1e2
  p = 1e6
  T = 300
  vel = -2
  snes_test_err = 1e-6
  generate_mesh = false
  use_transient_executioner = true
  fp_1phase = fp_1phase
[]

[FluidProperties]
  [./fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  [../]
[]

[Mesh]
  file = ../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[Variables]
  [./rho_br]
    family = SCALAR
    order = FIRST
  [../]
[]

[ICs]
  [./rho_br_ic]
    type = ScalarConstantIC
    variable = rho_br
    value = 1000
  [../]
[]

[ScalarKernels]
  [./mass]
    type = MassBalanceScalarKernel
    variable = rho_br
    boundary = '2 3'
    normals = '1 -1'
    rhoA = rhoA
    rhouA = rhouA
  [../]
[]

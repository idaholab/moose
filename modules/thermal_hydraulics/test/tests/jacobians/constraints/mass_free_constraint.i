[JacobianTest1Phase]
  A = 1e2
  p = 1e6
  T = 300
  vel = -2
  snes_test_err = 1e-6
  generate_mesh = false
  fp_1phase = fp_1phase
[]

[FluidProperties]
  [fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Mesh]
  file = ../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[Constraints]
  [mass]
    type = MassFreeConstraint
    variable = rhoA
    nodes = '1 2'
    normals = '1 -1'
    rhouA = rhouA
  []
[]

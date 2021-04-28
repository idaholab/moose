[JacobianTest1Phase]
  A = area_fn
  p = 1e6
  T = 300
  vel = 2
  snes_test_err = 1e-8
  generate_mesh = false
  fp_1phase = fp_1phase
[]

[FluidProperties]
  [fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Mesh]
  file = ../meshes/skew_1elem.e
  construct_side_list_from_node_list = true
[]

[Functions]
  [area_fn]
    type = PiecewiseLinear
    axis = x
    x = '-1  2'
    y = ' 2  1'
  []
[]

[Kernels]
  [test]
    type = OneD3EqnMomentumAreaGradient
    variable = rhouA
    arhoA = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
    A = A
    direction = direction
    p = p
  []
[]

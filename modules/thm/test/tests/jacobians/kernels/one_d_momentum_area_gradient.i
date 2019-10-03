[JacobianTest2Phase]
  A = area_fn
  alpha_vapor = 0.3
  p_liquid = 1e6
  p_vapor = 1e6
  T_liquid = 300
  T_vapor = 500
  vel_liquid = 2
  vel_vapor = 2
  snes_test_err = 1e-8
  generate_mesh = false
  fp_2phase = fp_2phase
[]

[FluidProperties]
  [./fp_2phase]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Mesh]
  file = ../meshes/skew_1elem.e
  construct_side_list_from_node_list = true
[]

[Functions]
  [./area_fn]
    type = PiecewiseLinear
    axis = x
    x = '-1  2'
    y = ' 2  1'
  [../]
[]

[Kernels]
  [./test]
    type = OneDMomentumAreaGradient
    variable = arhouA_liquid
    arhoA = arhoA_liquid
    arhouA = arhouA_liquid
    arhoEA = arhoEA_liquid
    A = A
    alpha = alpha_liquid
    direction = direction
    p = p_liquid
    beta = beta
  [../]
[]

[JacobianTest1Phase]
  A = 1
  p = 1e5
  T = 300
  vel = 2
  snes_test_err = 1e-1
  generate_mesh = false
  fp_1phase = fp_1phase
[]

[Mesh]
  file = ../meshes/skew_1elem.e
  construct_side_list_from_node_list = true
[]

[FluidProperties]
  [./fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  [../]
[]

[Kernels]
  [./mass_flux]
    type = OneDMassFlux
    variable = rhoA
    A = A
    arhoA = rhoA
    arhouA = rhouA
    direction = direction
    alpha = unity
    rho = rho
    vel = vel
  [../]
[]

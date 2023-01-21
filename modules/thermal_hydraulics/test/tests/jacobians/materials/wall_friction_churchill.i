# Tests the derivatives of WallFrictionChurchillMaterial

[JacobianTest1Phase]
  A = 10
  p = 1e5
  T = 300
  vel = 2
  D_h = 3.57
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

[Materials]
  [f_wall_mat]
    type = WallFrictionChurchillMaterial
    f_D = f_D
    rhoA  = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    mu = mu
    rho = rho
    vel = vel
    D_h = D_h
    roughness = 1
  []
[]

[Kernels]
  [test_kernel]
    type = MaterialDerivativeTestKernel
    variable = rhoA
    material_property = f_D
    coupled_variables = 'rhoA rhouA rhoEA'
  []
[]

[JacobianTest1Phase]
  A = 1
  p = 1e6
  T = 300
  vel = 2
  aux_variable_names = 'tw phf'
  aux_variable_values = '310 1'
  snes_test_err = 1e-9
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

[Materials]
  [const_mat_props]
    type = GenericConstantMaterial
    prop_names = 'htc'
    prop_values = '1e4'
  []
[]

[Kernels]
  [mom_flux]
    type = OneDEnergyWallHeating
    variable = rhoEA
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    T_wall = tw
    Hw = htc
    P_hf = phf
    T = T
  []
[]

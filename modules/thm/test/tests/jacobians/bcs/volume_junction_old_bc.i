[GlobalParams]
  gravity_magnitude = 9.81
[]

[JacobianTest1Phase]
  A = 1e2
  p = 1e6
  T = 300
  vel = -2
  snes_test_err = 1e-8
  generate_mesh = false
  use_transient_executioner = true
  fp_1phase = fp_1phase
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

[Mesh]
  file = ../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[Variables]
  [./br_rho]
    family = SCALAR
    order = FIRST
  [../]
  [./br_rhoe]
    family = SCALAR
    order = FIRST
  [../]
  [./br_vel]
    family = SCALAR
    order = FIRST
  [../]
[]

[AuxVariables]
  [./br_pressure]
    family = SCALAR
    order = FIRST
  [../]

  [./br_tot_mfr_in]
    family = SCALAR
    order = FIRST
  [../]
[]

[ICs]
  [./br_rho_ic]
    type = ScalarConstantIC
    variable = br_rho
    value = 996.960022
  [../]

  [./br_rhoe_ic]
    type = ScalarConstantIC
    variable = br_rhoe
    value = 112137074.87842458
  [../]

  [./br_vel_ic]
    type = ScalarConstantIC
    variable = br_vel
    value = 0
  [../]
[]

[AuxScalarKernels]
  [./br_pressure_aux]
    type = VolumeJunctionOldPressureAux
    variable = br_pressure
    junction_rho = br_rho
    junction_rhoe = br_rhoe
    fp = fp_1phase
  [../]

  [./tot_mfr_in_ak]
    type = TotalMassFlowRateIntoJunctionAux
    variable = br_tot_mfr_in
    nodes = '1 2'
    rhouA = rhouA
    normals = '-1 1'
  [../]
[]

[BCs]
  [./bc_1]
    type = VolumeJunctionOldBC
    variable = rhouA
    boundary = 2
    normal = 1
    eqn_name = MOMENTUM
    K = 2
    K_reverse = 3
    deltaH = 0.1
    ref_area = 1
    A = A
    rho = rho
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    vel = vel
    rho_junction = br_rho
    rhoe_junction = br_rhoe
    vel_junction = br_vel
    p_junction = br_pressure
    total_mfr_in = br_tot_mfr_in
    fp = fp_1phase
  [../]

  [./bc_2]
    type = VolumeJunctionOldBC
    variable = rhoEA
    boundary = 2
    normal = 1
    eqn_name = ENERGY
    K = 2
    K_reverse = 3
    deltaH = 0.1
    ref_area = 1
    A = A
    rho = rho
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    vel = vel
    rho_junction = br_rho
    rhoe_junction = br_rhoe
    vel_junction = br_vel
    p_junction = br_pressure
    total_mfr_in = br_tot_mfr_in
    fp = fp_1phase
  [../]
[]

[ScalarKernels]
  [./br_mass]
    type = MassBalanceScalarKernel
    variable = br_rho
    boundary = '2 3'
    normals = '1 -1'
    rhoA = rhoA
    rhouA = rhouA
  [../]

  [./br_mom]
    type = MomentumBalanceScalarKernel
    variable = br_vel
    boundary = '2 3'
    normals = '1 -1'
    ref_area = 3.14
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    vel = vel
    junction_rho = br_rho
    total_mfr_in = br_tot_mfr_in
  [../]

  [./br_erg]
    type = EnergyBalanceScalarKernel
    variable = br_rhoe
    boundary = '2 3'
    normals = '1 -1'
    A = A
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    p = p
    vel = vel
    e = e
    v = v
    fp = fp_1phase
  [../]
[]

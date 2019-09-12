[JacobianTest1Phase]
  A = 1e2
  p = 1e5
  T = 300
  vel = 2
  snes_test_err = 1e-7
  generate_mesh = false
  fp_1phase = fp_1phase
[]

[FluidProperties]
  [fp_1phase]
    type = LinearTestFluidProperties
  [../]
[]

[Mesh]
  file = ../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[AuxVariables]
  [./tmfri_junction]
    family = SCALAR
    order = FIRST
  [../]
  [./tieri_junction]
    family = SCALAR
    order = FIRST
  [../]
[]

[AuxScalarKernels]
  [./nmi_ak]
    type = TotalMassFlowRateIntoJunctionAux
    variable = tmfri_junction
    rhouA = rhouA
    nodes = '1 2'
    normals = '1 -1'
  [../]

  [./ntei_ak]
    type = TotalInternalEnergyRateIntoJunctionAux
    variable = tieri_junction
    nodes = '1 2'
    normals = '1 -1'
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
  [../]
[]

[Variables]
  [./p_junction]
    family = SCALAR
    order = FIRST
  [../]

  [./e_junction]
    family = SCALAR
    order = FIRST
  [../]
[]

[ICs]
  [./pbr_ic]
    type = ScalarConstantIC
    variable = p_junction
    value = 1e5
  [../]

  [./ebr_ic]
    type = ScalarConstantIC
    variable = e_junction
    value = 112563.477841489337152
  [../]
[]

[ScalarKernels]
  [./p_sk]
    type = OldJunctionMassBalanceScalarKernel
    variable = p_junction
    boundary = '2 3'
    normals = '1 -1'
    rhoA = rhoA
    rhouA = rhouA
  [../]

  [./e_sk]
    type = EnergyScalarKernel
    variable = e_junction
    boundary = '2 3'
    normals = '1 -1'
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    A = A
    vel = vel
    total_mfr_in = tmfri_junction
    total_int_energy_rate_in = tieri_junction
  [../]
[]

[Constraints]
  [./mass]
    type = OldJunctionEnergyConstraint
    variable = rhoEA
    nodes = '1 2'
    normals = '1 -1'
    K = '3 5'
    K_reverse = '11 13'
    A = A
    p_junction = p_junction
    energy_junction = e_junction
    rho = rho
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    initial_rho = 1000
    ref_area = 3.14
    vel = vel
    total_mfr_in = tmfri_junction
    total_int_energy_rate_in = tieri_junction
    fp = fp_1phase
  [../]
[]

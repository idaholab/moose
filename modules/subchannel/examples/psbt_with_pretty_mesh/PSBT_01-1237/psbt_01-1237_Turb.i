################################################################################
# Inputs relevant to the subchannel solver
################################################################################

[Mesh]
  type = SubchannelMesh
  nx = 6 #subchannel number in the x direction
  ny = 6 #subchannel number in the y direction
  max_dz = 0.02
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095 #means the half gap between sub_channel assemblies
  heated_length = 3.658
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [SumWijh]
  []
  [SumWijPrimeDhij]
  []
  [SumWijPrimeDUij]
  []
  [P]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [S]
  []
  [Sij]
  []
  [w_perim]
  []
  [q_prime]
  []
[]

[UserObjects]
  [subchannel_solver]
    type = SubchannelSolver
    execute_on = "INITIAL"
    mdot = mdot
    SumWij = SumWij
    SumWijh = SumWijh
    SumWijPrimeDhij = SumWijPrimeDhij
    SumWijPrimeDUij = SumWijPrimeDUij
    P = P
    h = h
    T = T
    rho = rho
    flow_area = S
    cross_flow_area = Sij
    wetted_perimeter = w_perim
    q_prime = q_prime
    T_in = 359.15 # K
    P_out = 4.923 # MPa
    mflux_in = 17.00 # 10^6 kg/m^2 Hr
  []
[]

[ICs]
  [S_IC]
    type = PsbtFlowAreaIC
    variable = S
  []
  [w_perim_IC]
    type = PsbtWettedPerimIC
    variable = w_perim
  []
  [q_prime_IC]
    type = PsbtPowerIC
    variable = q_prime
    power = 3.44 #MW
    #power = 0.0 #MW
    filename = "power_profile.txt" #type in name of file that describes power profile
  []
[]

[Outputs]
  exodus = true
[]

################################################################################
# Stuff needed to make the program execute
################################################################################

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

[Variables]
  [dummy_var]
  []
[]

[Kernels]
  [dummy_kern]
    type = Diffusion
    variable = dummy_var
  []
[]

[BCs]
  [dummy_bc1]
    variable = dummy_var
    boundary = 'bottom'
    type = DirichletBC
    value = 0
  []
  [dummy_bc2]
    variable = dummy_var
    boundary = 'top'
    type = DirichletBC
    value = 1
  []
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = "sub0.i"
    execute_on = "NONLINEAR"
  []
[]

[Transfers]
  [xfer_mdot]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = mdot
    variable = mdot
  []
  [xfer_SumWij]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = SumWij
    variable = SumWij
  []
  [xfer_SumWijh]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = SumWijh
    variable = SumWijh
  []
  [xfer_SumWijPrimeDhij]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = SumWijPrimeDhij
    variable = SumWijPrimeDhij
  []
  [xfer_SumWijPrimeDUij]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = SumWijPrimeDUij
    variable = SumWijPrimeDUij
  []
  [xfer_P]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = P
    variable = P
  []
  [xfer_h]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = h
    variable = h
  []
  [xfer_T]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = T
    variable = T
  []
  [xfer_rho]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = rho
    variable = rho
  []
  [q_prime]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = q_prime
    variable = q_prime
  []
[]

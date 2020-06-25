[Mesh]
  type = SubChannelMesh
  nx = 6
  ny = 6
  max_dz = 0.02
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095 # the half gap between sub-channel assemblies
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

[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]

[Problem]
  type = SubChannel1PhaseProblem
  T_in = 359.15 # K
  P_out = 4.923e6 # Pa
  mflux_in = ${fparse 1e+6 * 17.00 / 3600.} #Inlet coolant mass flux [1e+6 kg/m^2-hour] turns into kg/m^2-sec
  fp = water
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
    power = 3.44e6 # W
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

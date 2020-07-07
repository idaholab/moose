length = 3.658

[Mesh]
  type = SubChannelMesh
  nx = 6
  ny = 6
  max_dz = 0.02
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095
  heated_length = ${length}
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
  T_in = 359.15
  P_out = 4.923e6
  mflux_in = ${fparse 1e+6 * 17.00 / 3600.}
  fp = water
[]

[ICs]
  [S_IC]
    type = FlowAreaIC
    variable = S
  []
  [w_perim_IC]
    type = WettedPerimIC
    variable = w_perim
  []
  [q_prime_IC]
    type = PowerIC
    variable = q_prime
    power = 3.44e6
    filename = "power_profile.txt"
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [prettyMesh]
    type = FullSolveMultiApp
    input_files = "prettyMesh.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer_mdot]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = mdot
    variable = mdot
  []
  [xfer_SumWij]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = SumWij
    variable = SumWij
  []
  [xfer_SumWijh]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = SumWijh
    variable = SumWijh
  []
  [xfer_SumWijPrimeDhij]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = SumWijPrimeDhij
    variable = SumWijPrimeDhij
  []
  [xfer_SumWijPrimeDUij]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = SumWijPrimeDUij
    variable = SumWijPrimeDUij
  []
  [xfer_P]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = P
    variable = P
  []
  [xfer_h]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = h
    variable = h
  []
  [xfer_T]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = T
    variable = T
  []
  [xfer_rho]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = rho
    variable = rho
  []
  [xfer_q_prime]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = q_prime
    variable = q_prime
  []
[]

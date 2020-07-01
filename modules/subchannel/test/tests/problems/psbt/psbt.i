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

[GlobalParams]
  ######## Geometry #
  nx = 7
  ny = 3
  n_cells = 48
  n_blocks = 1
  pitch = 0.014605
  pin_diameter = 0.012065
  side_gap = 0.0015875
  heated_length = 1.2192
  spacer_z = '0.0'
  spacer_k = '0.0'
[]

######## BC's #################
T_in = 297.039 # K
P_out = 101325 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
  []

  [fuel_pins]
    type = SCMQuadPinMeshGenerator
    input = sub_channel
  []
[]

[AuxVariables]
  [mdot]
    block = sub_channel
  []
  [SumWij]
    block = sub_channel
  []
  [P]
    block = sub_channel
  []
  [DP]
    block = sub_channel
  []
  [h]
    block = sub_channel
  []
  [T]
    block = sub_channel
  []
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
  []
  [rho]
    block = sub_channel
  []
  [mu]
    block = sub_channel
  []
  [S]
    block = sub_channel
  []
  [w_perim]
    block = sub_channel
  []
  [q_prime]
    block = fuel_pins
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  fp = water
  beta = 0.006
  CT = 2.6
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  implicit = true
  segregated = false
  monolithic_thermal = false
  friction_closure = 'MATRA'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
[]

[ICs]
  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMQuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMQuadPowerIC
    variable = q_prime
    power = 5460 # W
    filename = "power_profile.txt"
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 0.012065
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []

  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
  []

  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = water
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = water
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = water
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = 131.43435930715006
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  csv = true
  [mdot_in_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_in.txt"
    height = 0.0
  []

  [rho_in_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = rho
    execute_on = final
    file_base = "rho_in.txt"
    height = 0.0
  []

  [mdot_out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_out.txt"
    height = 1.2192
  []

  [rho_out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = rho
    execute_on = final
    file_base = "rho_out.txt"
    height = 1.2192
  []

  [mdot_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_075.txt"
    height = 0.9144
  []

  [T_in_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "T_in.txt"
    height = 0.0
  []

  [T_out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "T_out.txt"
    height = 1.2192
  []
[]

[Postprocessors]
  [mdot7]
    type = SubChannelPointValue
    variable = mdot
    index = 7
    execute_on = 'initial timestep_end'
    height = 0.9144
  []
  [mdot8]
    type = SubChannelPointValue
    variable = mdot
    index = 8
    execute_on = 'initial timestep_end'
    height = 0.9144
  []
  [mdot9]
    type = SubChannelPointValue
    variable = mdot
    index = 9
    execute_on = 'initial timestep_end'
    height = 0.9144
  []
  [mdot10]
    type = SubChannelPointValue
    variable = mdot
    index = 10
    execute_on = 'initial timestep_end'
    height = 0.9144
  []
  [mdot11]
    type = SubChannelPointValue
    variable = mdot
    index = 11
    execute_on = 'initial timestep_end'
    height = 0.9144
  []
  [mdot12]
    type = SubChannelPointValue
    variable = mdot
    index = 12
    execute_on = 'initial timestep_end'
    height = 0.9144
  []
  [mdot13]
    type = SubChannelPointValue
    variable = mdot
    index = 13
    execute_on = 'initial timestep_end'
    height = 0.9144
  []
[]

[Executioner]
  type = Steady
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "3d.i"
    execute_on = "timestep_end"
  []
[]

###### Transfers to the detailedMesh at the end of the coupled simulations
[Transfers]
  [subchannel_transfer]
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
  [pin_transfer]
    type = SCMPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime'
  []
[]

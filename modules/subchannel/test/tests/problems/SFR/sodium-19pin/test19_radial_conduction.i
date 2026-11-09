# 19-pin liquid sodium assembly with only the center pin heated. Turbulent mixing is disabled
# (beta = 0, CT = 0), so heat reaches the outer subchannels through radial conduction across the
# gaps, plus diversion crossflow over the hydrodynamic development length.
T_in = 660
mass_flux_in = '${fparse 1e+6 * 300.00 / 36000.*0.5}'
P_out = 2.0e5 # Pa

[GlobalParams]
  nrings = 3
  n_cells = 5
  flat_to_flat = 0.056
  heated_length = 0.5
  pitch = 0.012
[]

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    pin_diameter = 0.01
    dwire = 0.002
    hwire = 0.0833
    spacer_z = '0'
    spacer_k = '5.0'
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = 2.0e5
  compute_density = true
  compute_viscosity = true
  compute_power = true
  implicit = true
  segregated = true
  verbose_subchannel = true
  pin_HTC_closure = 'gnielinski'
  friction_closure = 'Chen'
  mixing_closure = 'no_mixing'
[]

[SCMClosures]
  [Chen]
    type = SCMFrictionChenTodreas
  []
  [gnielinski]
    type = SCMHTCGnielinski
  []
  [no_mixing]
    type = SCMMixingConstantBeta
    beta = 0.0
    CT = 0.0
  []
[]

[ICs]
  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = 1000.0 # W
    filename = "pin_power_profile19_center.txt"
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 0.01
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []

  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = sodium
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
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  # Outlet temperatures from the center of the assembly outward
  [T_center]
    type = SubChannelPointValue
    variable = T
    index = 0
    execute_on = 'timestep_end'
    height = 0.5
  []
  [T_ring2_inner]
    type = SubChannelPointValue
    variable = T
    index = 6
    execute_on = 'timestep_end'
    height = 0.5
  []
  [T_ring2_outer]
    type = SubChannelPointValue
    variable = T
    index = 12
    execute_on = 'timestep_end'
    height = 0.5
  []
  [T_edge]
    type = SubChannelPointValue
    variable = T
    index = 30
    execute_on = 'timestep_end'
    height = 0.5
  []
  [T_corner]
    type = SubChannelPointValue
    variable = T
    index = 41
    execute_on = 'timestep_end'
    height = 0.5
  []
  [Mean_Temp]
    type = SCMPlanarMean
    variable = T
    height = 0.5
  []
  [Total_power_SCMTHPowerPostprocessor]
    type = SCMTHPowerPostprocessor
  []
[]

[Executioner]
  type = Steady
[]

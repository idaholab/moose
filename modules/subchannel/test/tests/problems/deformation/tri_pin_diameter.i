T_in = 588.5
P_out = 2.0e5
flow_area = 0.0004980799633447909
mass_flux_in = '${fparse 55*3.78541/10/60/flow_area}'

[TriSubChannelMesh]
  [assembly]
    type = SCMTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 5
    flat_to_flat = 3.41e-2
    heated_length = 0.5
    pin_diameter = 5.5e-3
    pitch = 7.26e-3
    dwire = 1.42e-3
    hwire = 0.3048
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
  P_out = ${P_out}
  compute_density = true
  compute_viscosity = true
  compute_power = false
  implicit = true
  segregated = false
  interpolation_scheme = upwind
  friction_closure = 'cheng'
  mixing_closure = 'cheng_todreas'
  pin_HTC_closure = 'gnielinski'
  full_output = true
[]

[SCMClosures]
  [cheng]
    type = SCMFrictionUpdatedChengTodreas
  []
  [gnielinski]
    type = SCMHTCGnielinski
  []
  [cheng_todreas]
    type = SCMMixingChengTodreas
    CT = 2.6
  []
[]

[ICs]
  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 5.84e-3
  []
  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
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

[Postprocessors]
  [S_deformed]
    type = SubChannelPointValue
    variable = S
    index = 0
    height = 0.25
    execute_on = 'timestep_end'
  []
  [w_perim_deformed]
    type = SubChannelPointValue
    variable = w_perim
    index = 0
    height = 0.25
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  console = true
  csv = true
[]

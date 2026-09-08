T_in = 624.70556
P_out = 2.0e5
mass_flux_in = '${fparse 2.45 / 0.000854322}'

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    nrings = 5
    n_cells = 2
    flat_to_flat = 0.0464
    heated_length = 0.343
    pin_diameter = 0.004419
    pitch = 0.005664
    dwire = 0.003
    hwire = 0.1524
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
  friction_closure = cheng
  mixing_closure = cheng_todreas
  pin_HTC_closure = gnielinski
[]

[SCMClosures]
  [cheng]
    type = SCMFrictionUpdatedChengTodreas
  []
  [cheng_todreas]
    type = SCMMixingChengTodreas
    CT = 2.6
  []
  [gnielinski]
    type = SCMHTCGnielinski
  []
[]

[ICs]
  [pin_diameter]
    type = ConstantIC
    variable = Dpin
    value = 0.004419
  []
  [temperature]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []
  [pressure]
    type = ConstantIC
    variable = P
    value = 0
  []
  [viscosity]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = sodium
  []
  [density]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = sodium
  []
  [enthalpy]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = sodium
  []
  [mass_flow_rate]
    type = ConstantIC
    variable = mdot
    value = 0
  []
[]

[AuxKernels]
  [inlet_temperature]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = timestep_begin
  []
  [inlet_mass_flow_rate]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = timestep_begin
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
[]

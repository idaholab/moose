# Radial heat conduction verification in a 7-pin triangular assembly. The pins are unheated and
# bare, turbulent mixing is disabled (beta = 0, CT = 0), and a large lateral form loss
# coefficient Kij suppresses diversion crossflow. The duct heats the subchannels on one side of
# the assembly and cools those on the other side with the same total power, so heat crosses the
# assembly only by radial conduction through the gaps. The axial flow is low so that the
# temperature differences between subchannels are large. The outlet subchannel temperatures are
# compared with an independent solution of the same gap conductance network in
# test_radial_conduction.py.
T_in = 660 # K
mass_flux_in = 5.0 # kg/m2/s
P_out = 2.0e5 # Pa
duct_flux = 2.0e4 # W/m2

[GlobalParams]
  nrings = 2
  n_cells = 20
  flat_to_flat = 0.035
  heated_length = 0.5
  pitch = 0.012
[]

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    pin_diameter = 0.01
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0'
    spacer_k = '0'
    Kij = 1e8
  []
  [duct]
    type = SCMTriDuctMeshGenerator
    input = subchannel
  []
[]

# Constant properties close to sodium at 660 K, so that the network solution has constant
# conductances. With cp = cv, zero thermal expansion and porepressure_coefficient = 0,
# h = cp * T exactly
[FluidProperties]
  [sodium]
    type = SimpleFluidProperties
    density0 = 850
    thermal_expansion = 0
    cp = 1270
    cv = 1270
    thermal_conductivity = 70
    viscosity = 2.5e-4
    porepressure_coefficient = 0
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  P_out = ${P_out}
  compute_density = false
  compute_viscosity = false
  compute_power = true
  implicit = true
  segregated = true
  rtol = 1e-10
  atol = 1e-10
  friction_closure = 'Chen'
  mixing_closure = 'no_mixing'
  pin_HTC_closure = 'gnielinski'
  duct_HTC_closure = 'gnielinski'
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

[Functions]
  # Heat the duct where x > 0 and cool it where x < 0. The duct nodes on x = 0 get no flux, so
  # the heated and cooled sides receive the same power
  [duct_flux_fn]
    type = ParsedFunction
    expression = 'if(x > 1e-6, ${duct_flux}, if(x < -1e-6, -${duct_flux}, 0))'
  []
[]

[ICs]
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
  [mu_ic]
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
  [q_prime_ic]
    type = ConstantIC
    variable = q_prime
    value = 0.0
  []
  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 0.01
  []
  [duct_heat_flux_ic]
    type = FunctionIC
    variable = duct_heat_flux
    function = duct_flux_fn
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

[VectorPostprocessors]
  # Sampled at every axial level for the independent network solution in
  # test_radial_conduction.py
  [channels]
    type = NodalValueSampler
    variable = 'T mdot S'
    block = subchannel
    sort_by = id
    execute_on = timestep_end
  []
  [duct]
    type = NodalValueSampler
    variable = 'duct_heat_flux'
    block = duct
    sort_by = id
    execute_on = timestep_end
  []
[]

[Postprocessors]
  [Total_power_SCMTHPowerPostprocessor]
    type = SCMTHPowerPostprocessor
  []
  [Total_power_SCMDuctPowerPostprocessor]
    type = SCMDuctHeatRatePostprocessor
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

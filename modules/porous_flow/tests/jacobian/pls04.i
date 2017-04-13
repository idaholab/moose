# PorousFlowPiecewiseLinearSink with 2-phase, 3-components, with enthalpy, internal_energy, and thermal_conductivity
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 2
  nz = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./ppwater]
  [../]
  [./ppgas]
  [../]
  [./massfrac_ph0_sp0]
  [../]
  [./massfrac_ph0_sp1]
  [../]
  [./massfrac_ph1_sp0]
  [../]
  [./massfrac_ph1_sp1]
  [../]
  [./temp]
  [../]
[]


[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp ppwater ppgas massfrac_ph0_sp0 massfrac_ph0_sp1 massfrac_ph1_sp0 massfrac_ph1_sp1'
    number_fluid_phases = 2
    number_fluid_components = 3
  [../]
[]

[ICs]
  [./temp]
    type = RandomIC
    variable = temp
    min = 1
    max = 2
  [../]
  [./ppwater]
    type = RandomIC
    variable = ppwater
    min = -1
    max = 0
  [../]
  [./ppgas]
    type = RandomIC
    variable = ppgas
    min = 0
    max = 1
  [../]
  [./massfrac_ph0_sp0]
    type = RandomIC
    variable = massfrac_ph0_sp0
    min = 0
    max = 1
  [../]
  [./massfrac_ph0_sp1]
    type = RandomIC
    variable = massfrac_ph0_sp1
    min = 0
    max = 1
  [../]
  [./massfrac_ph1_sp0]
    type = RandomIC
    variable = massfrac_ph1_sp0
    min = 0
    max = 1
  [../]
  [./massfrac_ph1_sp1]
    type = RandomIC
    variable = massfrac_ph1_sp1
    min = 0
    max = 1
  [../]
[]

[Kernels]
  [./dummy_temp]
    type = TimeDerivative
    variable = temp
  [../]
  [./dummy_ppwater]
    type = TimeDerivative
    variable = ppwater
  [../]
  [./dummy_ppgas]
    type = TimeDerivative
    variable = ppgas
  [../]
  [./dummy_m00]
    type = TimeDerivative
    variable = massfrac_ph0_sp0
  [../]
  [./dummy_m01]
    type = TimeDerivative
    variable = massfrac_ph0_sp1
  [../]
  [./dummy_m10]
    type = TimeDerivative
    variable = massfrac_ph1_sp0
  [../]
  [./dummy_m11]
    type = TimeDerivative
    variable = massfrac_ph1_sp1
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = temp
    at_nodes = true
  [../]
  [./ppss_nodal]
    type = PorousFlow2PhasePP_VG
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    at_nodes = true
    al = 1
    m = 0.5
  [../]
  [./ppss]
    type = PorousFlow2PhasePP_VG
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    at_nodes = false
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph0_sp1 massfrac_ph1_sp0 massfrac_ph1_sp1'
    at_nodes = true
  [../]
  [./dens0_nodal]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1.5
    phase = 0
  [../]
  [./dens1_nodal]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 0.5
    bulk_modulus = 0.5
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1
    phase = 0
    at_nodes = true
  [../]
  [./visc1]
    type = PorousFlowViscosityConst
    viscosity = 1.4
    phase = 1
    at_nodes = true
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_nodal
    at_nodes = true
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    at_nodes = false
    permeability = '1 0 0 0 2 0 0 0 3'
  [../]
  [./relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    at_nodes = true
  [../]
  [./relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
    at_nodes = true
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability_nodal
    at_nodes = true
  [../]
  [./enthalpy0]
    type = PorousFlowEnthalpy
    phase = 0
    at_nodes = true
  [../]
  [./enthalpy1]
    type = PorousFlowEnthalpy
    phase = 1
    at_nodes = true
  [../]
  [./enthalpy_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_enthalpy_nodal
    at_nodes = true
  [../]
  [./fluid_energy0_nodal]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 1.1
    phase = 0
    at_nodes = true
  [../]
  [./fluid_energy1_nodal]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 1.8
    phase = 1
    at_nodes = true
  [../]
  [./energy_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
    at_nodes = true
  [../]
  [./thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    at_nodes = false
    dry_thermal_conductivity = '0.1 0.2 0.3 0.2 0 0.1 0.3 0.1 0.1'
    wet_thermal_conductivity = '10 2 31 2 40 1 31 1 10'
    exponent = 0.5
  [../]
[]

[BCs]
  [./flux_w]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'left'
    pt_vals = '-1 -0.5 0'
    multipliers = '1 2 4'
    variable = ppwater
    mass_fraction_component = 0
    fluid_phase = 0
    use_relperm = true
    use_mobility = true
    use_enthalpy = true
    flux_function = 'x*y'
  [../]
  [./flux_g]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'top'
    pt_vals = '0 0.5 1'
    multipliers = '1 -2 4'
    mass_fraction_component = 0
    variable = ppgas
    fluid_phase = 1
    use_relperm = true
    use_mobility = true
    use_internal_energy = true
    flux_function = '-x*y'
  [../]
  [./flux_1]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'right'
    pt_vals = '0 0.5 1'
    multipliers = '1 3 4'
    mass_fraction_component = 1
    variable = massfrac_ph0_sp0
    fluid_phase = 0
    use_relperm = true
    use_mobility = true
    use_internal_energy = true
  [../]
  [./flux_2]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'back top'
    pt_vals = '0 0.5 1'
    multipliers = '0 1 -3'
    mass_fraction_component = 1
    variable = massfrac_ph1_sp0
    fluid_phase = 1
    use_relperm = true
    use_mobility = true
    use_enthalpy = true
    flux_function = '0.5*x*y'
  [../]
  [./flux_3]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'right'
    pt_vals = '0 0.5 1'
    multipliers = '1 3 4'
    mass_fraction_component = 2
    variable = ppwater
    fluid_phase = 0
    use_relperm = true
    use_enthalpy = true
    use_mobility = true
  [../]
  [./flux_4]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'back top'
    pt_vals = '0 0.5 1'
    multipliers = '0 1 -3'
    mass_fraction_component = 2
    variable = massfrac_ph1_sp0
    fluid_phase = 1
    use_relperm = true
    use_mobility = true
    flux_function = '-0.5*x*y'
    use_enthalpy = true
    use_thermal_conductivity = true
  [../]
[]


[Preconditioning]
  [./check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  file_base = pls04
[]

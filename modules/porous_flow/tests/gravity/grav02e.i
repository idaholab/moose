# Checking that gravity head is established in the transient situation when 0<=saturation<=1 (note the less-than-or-equal-to).
# 2phase (PS), 2components, constant capillary pressure, constant fluid bulk-moduli for each phase, constant viscosity,
# constant permeability, Corey relative permeabilities with no residual saturation

[Mesh]
  type = GeneratedMesh
  dim = 2
  ny = 10
  ymax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 -10 0'
[]

[Variables]
  [./ppwater]
    initial_condition = 1.5e6
  [../]
  [./sgas]
    initial_condition = 0.3
  [../]
[]

[AuxVariables]
  [./massfrac_ph0_sp0]
    initial_condition = 1
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0
  [../]
  [./ppgas]
    family = MONOMIAL
    order = FIRST
  [../]
  [./swater]
    family = MONOMIAL
    order = FIRST
  [../]
  [./relpermwater]
    family = MONOMIAL
    order = FIRST
  [../]
  [./relpermgas]
    family = MONOMIAL
    order = FIRST
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  [../]
[]

[AuxKernels]
  [./ppgas]
    type = PorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = ppgas
  [../]
  [./swater]
    type = PorousFlowPropertyAux
    property = saturation
    phase = 0
    variable = swater
  [../]
  [./relpermwater]
    type = PorousFlowPropertyAux
    property = relperm
    phase = 0
    variable = relpermwater
  [../]
  [./relpermgas]
    type = PorousFlowPropertyAux
    property = relperm
    phase = 1
    variable = relpermgas
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./ppss]
    type = PorousFlow2PhasePS
    at_nodes = true
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    pc = -1e5
  [../]
  [./ppss_qp]
    type = PorousFlow2PhasePS
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    pc = -1e5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 2e9
    phase = 0
  [../]
  [./dens1]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 10
    bulk_modulus = 2e9
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    include_old = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1000
    bulk_modulus = 2e9
    phase = 0
  [../]
  [./dens1_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 10
    bulk_modulus = 2e9
    phase = 1
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1e-3
    phase = 0
  [../]
  [./visc1]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 1e-5
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-11 0 0 0 1e-11 0  0 0 1e-11'
  [../]
  [./relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 2
    phase = 0
  [../]
  [./relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 2
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./relperm_water_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  [../]
  [./relperm_gas_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  [../]
  [./relperm_all_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability_qp
  [../]
[]

[Postprocessors]
  [./mass_ph0]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  [../]
  [./mass_ph1]
    type = PorousFlowFluidMass
    fluid_component = 1
    execute_on = 'initial timestep_end'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1e5
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e4
  [../]
[]

[Outputs]
  execute_on = 'initial timestep_end'
  file_base = grav02e
  exodus = true
  print_perf_log = true
  csv = false
[]

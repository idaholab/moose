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
  [ppwater]
    initial_condition = 1.5e6
  []
  [sgas]
    initial_condition = 0.3
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
  [ppgas]
    family = MONOMIAL
    order = FIRST
  []
  [swater]
    family = MONOMIAL
    order = FIRST
  []
  [relpermwater]
    family = MONOMIAL
    order = FIRST
  []
  [relpermgas]
    family = MONOMIAL
    order = FIRST
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  []
[]

[AuxKernels]
  [ppgas]
    type = PorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = ppgas
  []
  [swater]
    type = PorousFlowPropertyAux
    property = saturation
    phase = 0
    variable = swater
  []
  [relpermwater]
    type = PorousFlowPropertyAux
    property = relperm
    phase = 0
    variable = relpermwater
  []
  [relpermgas]
    type = PorousFlowPropertyAux
    property = relperm
    phase = 1
    variable = relpermgas
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 1e5
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    viscosity = 1e-3
    thermal_expansion = 0
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 10
    viscosity = 1e-5
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-11 0 0 0 1e-11 0  0 0 1e-11'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  []
[]

[Postprocessors]
  [mass_ph0]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [mass_ph1]
    type = PorousFlowFluidMass
    fluid_component = 1
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol'
    petsc_options_value = 'bcgs bjacobi 1E-12 1E-10'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1e5
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e4
  []
[]

[Outputs]
  execute_on = 'initial timestep_end'
  file_base = grav02e
  exodus = true
  perf_graph = true
  csv = false
[]

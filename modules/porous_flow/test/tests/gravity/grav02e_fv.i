# Checking that gravity head is established in the transient situation when 0<=saturation<=1 (note the less-than-or-equal-to).
# 2phase (PS), 2components, constant capillary pressure, constant fluid bulk-moduli for each phase, constant viscosity,
# constant permeability, Corey relative permeabilities with no residual saturation

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '-10 0 0'
[]

[Variables]
  [ppwater]
    type = MooseVariableFVReal
    initial_condition = 1.5e6
  []
  [sgas]
    type = MooseVariableFVReal

    initial_condition = 0.3
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    type = MooseVariableFVReal
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    type = MooseVariableFVReal
    initial_condition = 0
  []
  [ppgas]
    type = MooseVariableFVReal
  []
  [swater]
    type = MooseVariableFVReal
  []
  [relpermwater]
    type = MooseVariableFVReal
  []
  [relpermgas]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  []
  [flux0]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux1]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  []
[]

[AuxKernels]
  [ppgas]
    type = ADPorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = ppgas
    execute_on = 'initial timestep_end'
  []
  [swater]
    type = ADPorousFlowPropertyAux
    property = saturation
    phase = 0
    variable = swater
    execute_on = 'initial timestep_end'
  []
  [relpermwater]
    type = ADPorousFlowPropertyAux
    property = relperm
    phase = 0
    variable = relpermwater
    execute_on = 'initial timestep_end'
  []
  [relpermgas]
    type = ADPorousFlowPropertyAux
    property = relperm
    phase = 1
    variable = relpermgas
    execute_on = 'initial timestep_end'
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
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow2PhasePS
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = ADPorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [simple_fluid0]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1e-11 0 0 0 1e-11 0  0 0 1e-11'
  []
  [relperm_water]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm_gas]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  []
[]

[VectorPostprocessors]
  [vars]
    type = ElementValueSampler
    variable = 'ppgas ppwater sgas swater'
    sort_by = x
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 5e3
  nl_abs_tol = 1e-12
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e3
  []
[]

[Outputs]
  execute_on = 'final'
  perf_graph = true
  csv = true
[]

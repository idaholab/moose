# Checking the Jacobian of Flux-Limited TVD Advection, 2 phases, 2 components, using flux_limiter_type = None
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmin = 0
  xmax = 1
  ny = 2
  ymin = -1
  ymax = 2
  bias_y = 1.5
[]

[GlobalParams]
  gravity = '1 2 -0.5'
  PorousFlowDictator = dictator
[]

[Variables]
  [ppwater]
  []
  [ppgas]
  []
  [massfrac_ph0_sp0]
  []
  [massfrac_ph1_sp0]
  []
[]


[ICs]
  [ppwater]
    type = RandomIC
    variable = ppwater
    min = -1
    max = 0
  []
  [ppgas]
    type = RandomIC
    variable = ppgas
    min = 0
    max = 1
  []
  [massfrac_ph0_sp0]
    type = RandomIC
    variable = massfrac_ph0_sp0
    min = 0
    max = 1
  []
  [massfrac_ph1_sp0]
    type = RandomIC
    variable = massfrac_ph1_sp0
    min = 0
    max = 1
  []
[]

[Kernels]
  [flux_ph0_sp0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = ppwater
    advective_flux_calculator = advective_flux_calculator_ph0_sp0
  []
  [flux_ph0_sp1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = ppgas
    advective_flux_calculator = advective_flux_calculator_ph0_sp1
  []
  [flux_ph1_sp0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = massfrac_ph0_sp0
    advective_flux_calculator = advective_flux_calculator_ph1_sp0
  []
  [flux_ph1_sp1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = massfrac_ph1_sp0
    advective_flux_calculator = advective_flux_calculator_ph1_sp1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1.5
    density0 = 1
    thermal_expansion = 0
    viscosity = 1
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 0.5
    thermal_expansion = 0
    viscosity = 1.4
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas massfrac_ph0_sp0 massfrac_ph1_sp0'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1
    m = 0.5
  []
  [advective_flux_calculator_ph0_sp0]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = None
    phase = 0
    fluid_component = 0
  []
  [advective_flux_calculator_ph0_sp1]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = None
    phase = 0
    fluid_component = 1
  []
  [advective_flux_calculator_ph1_sp0]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = None
    phase = 1
    fluid_component = 0
  []
  [advective_flux_calculator_ph1_sp1]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = None
    phase = 1
    fluid_component = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
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
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.21 0 0  0 1.5 0  0 0 0.8'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
  num_steps = 1
  dt = 1
[]

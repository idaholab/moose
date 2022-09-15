# Checking the Jacobian of Flux-Limited TVD Advection, 1 phase, 3 components, unsaturated, using flux_limiter_type = none
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  xmin = 0
  xmax = 1
  ny = 1
  ymin = -1
  ymax = 2
[]

[GlobalParams]
  gravity = '1 2 -0.5'
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
  [tracer0]
  []
  [tracer1]
  []
[]


[ICs]
  [pp]
    variable = pp
    type = RandomIC
    min = -1
    max = 0
  []
  [tracer0]
    variable = tracer0
    type = RandomIC
    min = 0
    max = 1
  []
  [tracer1]
    variable = tracer1
    type = RandomIC
    min = 0
    max = 1
  []
[]

[Kernels]
  [fluxpp]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = pp
    advective_flux_calculator = advective_flux_calculator_0
  []
  [flux0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = tracer0
    advective_flux_calculator = advective_flux_calculator_1
  []
  [flux1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = tracer1
    advective_flux_calculator = advective_flux_calculator_2
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 0.4
    viscosity = 1.1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp tracer0 tracer1'
    number_fluid_phases = 1
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1
    m = 0.5
  []
  [advective_flux_calculator_0]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = None
    fluid_component = 0
  []
  [advective_flux_calculator_1]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = None
    fluid_component = 1
  []
  [advective_flux_calculator_2]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    flux_limiter_type = None
    fluid_component = 2
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'tracer0 tracer1'
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    phase = 0
    n = 2
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

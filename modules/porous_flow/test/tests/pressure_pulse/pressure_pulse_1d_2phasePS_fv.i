# Pressure pulse in 1D with 2 phases, 2components - transient using FV

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmin = 0
    xmax = 100
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [ppwater]
    type = MooseVariableFVReal
    initial_condition = 2e6
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
    family = MONOMIAL
    order = CONSTANT
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
    variable = ppwater
    fluid_component = 0
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux1]
    type = FVPorousFlowAdvectiveFlux
    variable = sgas
    fluid_component = 1
  []
[]

[AuxKernels]
  [ppgas]
    type = ADPorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = ppgas
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
    thermal_expansion = 0
    viscosity = 1e-3
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 2e7
    density0 = 1
    thermal_expansion = 0
    viscosity = 1e-5
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
    permeability = '1e-15 0 0 0 1e-15 0 0 0 1e-15'
  []
  [relperm_water]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
  [relperm_gas]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  []
[]

[FVBCs]
  [leftwater]
    type = FVDirichletBC
    boundary = left
    value = 3e6
    variable = ppwater
  []
  [rightwater]
    type = FVDirichletBC
    boundary = right
    value = 2e6
    variable = ppwater
  []
  [sgas]
    type = FVDirichletBC
    boundary = 'left right'
    value = 0.3
    variable = sgas
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
  dt = 1e3
  end_time = 1e4
[]

[VectorPostprocessors]
  [pp]
    type = ElementValueSampler
    sort_by = x
    variable = 'ppwater ppgas'
  []
[]

[Outputs]
  file_base = pressure_pulse_1d_2phasePS_fv
  print_linear_residuals = false
  [csv]
    type = CSV
    execute_on = final
  []
  exodus = true
[]

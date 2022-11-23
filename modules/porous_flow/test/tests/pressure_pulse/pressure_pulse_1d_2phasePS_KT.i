# Pressure pulse in 1D with 2 phases, 2components - transient
# Using KT stabilization

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [ppwater]
    initial_condition = 2e6
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
[]

[Kernels]
  [mass_component0]
    type = PorousFlowMassTimeDerivative
    variable = ppwater
    fluid_component = 0
  []
  [flux_component0_phase0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = ppwater
    advective_flux_calculator = afc_component0_phase0
  []
  [flux_component0_phase1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = ppwater
    advective_flux_calculator = afc_component0_phase1
  []
  [mass_component1]
    type = PorousFlowMassTimeDerivative
    variable = sgas
    fluid_component = 1
  []
  [flux_component1_phase0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = sgas
    advective_flux_calculator = afc_component1_phase0
  []
  [flux_component1_phase1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = sgas
    advective_flux_calculator = afc_component1_phase1
  []
[]

[AuxKernels]
  [ppgas]
    type = PorousFlowPropertyAux
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
  [afc_component0_phase0]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    fluid_component = 0
    phase = 0
    flux_limiter_type = superbee
  []
  [afc_component0_phase1]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    fluid_component = 0
    phase = 1
    flux_limiter_type = superbee
  []
  [afc_component1_phase0]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    fluid_component = 1
    phase = 0
    flux_limiter_type = superbee
  []
  [afc_component1_phase1]
    type = PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
    fluid_component = 1
    phase = 1
    flux_limiter_type = superbee
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
    permeability = '1e-15 0 0 0 1e-15 0 0 0 1e-15'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 1
  []
[]

[BCs]
  [leftwater]
    type = DirichletBC
    boundary = left
    value = 3e6
    variable = ppwater
  []
  [rightwater]
    type = DirichletBC
    boundary = right
    value = 2e6
    variable = ppwater
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-20 10000'
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
    type = LineValueSampler
    warn_discontinuous_face_values = false
    sort_by = x
    variable = 'ppwater ppgas'
    start_point = '0 0 0'
    end_point = '100 0 0'
    num_points = 11
  []
[]

[Outputs]
  file_base = pressure_pulse_1d_2phasePS_KT
  print_linear_residuals = false
  [csv]
    type = CSV
    execute_on = final
  []
[]

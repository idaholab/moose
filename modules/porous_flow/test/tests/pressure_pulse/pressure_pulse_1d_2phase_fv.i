# Pressure pulse in 1D with 2 phases (with one having zero saturation), 2components - transient using FV
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
[]

[Variables]
  [ppwater]
    type = MooseVariableFVReal
    initial_condition = 2E6
  []
  [ppgas]
    type = MooseVariableFVReal
    initial_condition = 2E6
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
    gravity = '0 0 0'
    fluid_component = 0
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 1
    variable = ppgas
  []
  [flux1]
    type = FVPorousFlowAdvectiveFlux
    variable = ppgas
    gravity = '0 0 0'
    fluid_component = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
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
    bulk_modulus = 2e6
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
    type = ADPorousFlow2PhasePP
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
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
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
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
    value = 3E6
    variable = ppwater
  []
  [leftgas]
    type = FVDirichletBC
    boundary = left
    value = 3E6
    variable = ppgas
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E-12'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3
  end_time = 1E4
[]

[Postprocessors]
  [p005]
    type = PointValue
    variable = ppwater
    point = '5 0 0'
    execute_on = 'initial timestep_end'
  []
  [p015]
    type = PointValue
    variable = ppwater
    point = '15 0 0'
    execute_on = 'initial timestep_end'
  []
  [p025]
    type = PointValue
    variable = ppwater
    point = '25 0 0'
    execute_on = 'initial timestep_end'
  []
  [p035]
    type = PointValue
    variable = ppwater
    point = '35 0 0'
    execute_on = 'initial timestep_end'
  []
  [p045]
    type = PointValue
    variable = ppwater
    point = '45 0 0'
    execute_on = 'initial timestep_end'
  []
  [p055]
    type = PointValue
    variable = ppwater
    point = '55 0 0'
    execute_on = 'initial timestep_end'
  []
  [p065]
    type = PointValue
    variable = ppwater
    point = '65 0 0'
    execute_on = 'initial timestep_end'
  []
  [p075]
    type = PointValue
    variable = ppwater
    point = '75 0 0'
    execute_on = 'initial timestep_end'
  []
  [p085]
    type = PointValue
    variable = ppwater
    point = '85 0 0'
    execute_on = 'initial timestep_end'
  []
  [p095]
    type = PointValue
    variable = ppwater
    point = '95 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = pressure_pulse_1d_2phase_fv
  print_linear_residuals = false
  csv = true
[]

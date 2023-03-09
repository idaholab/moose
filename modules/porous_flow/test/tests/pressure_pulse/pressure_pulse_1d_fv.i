# Pressure pulse in 1D with 1 phase - transient FV model

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 2E6
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux]
    type = FVPorousFlowAdvectiveFlux
    variable = pp
    gravity = '0 0 0'
    fluid_component = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-7
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
    temperature = 293
  []
  [ppss]
    type = ADPorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityConst
    kr = 1
    phase = 0
  []
[]

[FVBCs]
  [left]
    type = FVPorousFlowAdvectiveFluxBC
    boundary = left
    porepressure_value = 3E6
    variable = pp
    gravity = '0 0 0'
    fluid_component = 0
    phase = 0
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
  dt = 1E3
  end_time = 1E4
[]

[Postprocessors]
  [p005]
    type = PointValue
    variable = pp
    point = '5 0 0'
    execute_on = 'initial timestep_end'
  []
  [p015]
    type = PointValue
    variable = pp
    point = '15 0 0'
    execute_on = 'initial timestep_end'
  []
  [p025]
    type = PointValue
    variable = pp
    point = '25 0 0'
    execute_on = 'initial timestep_end'
  []
  [p035]
    type = PointValue
    variable = pp
    point = '35 0 0'
    execute_on = 'initial timestep_end'
  []
  [p045]
    type = PointValue
    variable = pp
    point = '45 0 0'
    execute_on = 'initial timestep_end'
  []
  [p055]
    type = PointValue
    variable = pp
    point = '55 0 0'
    execute_on = 'initial timestep_end'
  []
  [p065]
    type = PointValue
    variable = pp
    point = '65 0 0'
    execute_on = 'initial timestep_end'
  []
  [p075]
    type = PointValue
    variable = pp
    point = '75 0 0'
    execute_on = 'initial timestep_end'
  []
  [p085]
    type = PointValue
    variable = pp
    point = '85 0 0'
    execute_on = 'initial timestep_end'
  []
  [p095]
    type = PointValue
    variable = pp
    point = '95 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = pressure_pulse_1d_fv
  print_linear_residuals = false
  csv = true
[]

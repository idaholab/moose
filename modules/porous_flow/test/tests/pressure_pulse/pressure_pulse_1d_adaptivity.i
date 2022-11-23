# Pressure pulse in 1D with 1 phase - transient simulation with a constant
# PorousFlowPorosity and mesh adaptivity with an indicator

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 100
[]

[Adaptivity]
  marker = marker
  [Markers]
    [marker]
      type = ErrorFractionMarker
      indicator = front
      refine = 0.5
      coarsen = 0.2
    []
  []
  [Indicators]
    [front]
      type = GradientJumpIndicator
      variable = pp
    []
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    initial_condition = 2E6
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux]
    type = PorousFlowAdvectiveFlux
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
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0
    phase = 0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    preset = false
    value = 3E6
    variable = pp
  []
  [right]
    type = PorousFlowPiecewiseLinearSink
    variable = pp
    boundary = right
    fluid_phase = 0
    pt_vals = '0 1E9'
    multipliers = '0 1E9'
    mass_fraction_component = 0
    use_mobility = true
    flux_function = 1E-6
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
  end_time = 5e3
[]

[Postprocessors]
  [p000]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  []
  [p010]
    type = PointValue
    variable = pp
    point = '10 0 0'
    execute_on = 'initial timestep_end'
  []
  [p020]
    type = PointValue
    variable = pp
    point = '20 0 0'
    execute_on = 'initial timestep_end'
  []
  [p030]
    type = PointValue
    variable = pp
    point = '30 0 0'
    execute_on = 'initial timestep_end'
  []
  [p040]
    type = PointValue
    variable = pp
    point = '40 0 0'
    execute_on = 'initial timestep_end'
  []
  [p050]
    type = PointValue
    variable = pp
    point = '50 0 0'
    execute_on = 'initial timestep_end'
  []
  [p060]
    type = PointValue
    variable = pp
    point = '60 0 0'
    execute_on = 'initial timestep_end'
  []
  [p070]
    type = PointValue
    variable = pp
    point = '70 0 0'
    execute_on = 'initial timestep_end'
  []
  [p080]
    type = PointValue
    variable = pp
    point = '80 0 0'
    execute_on = 'initial timestep_end'
  []
  [p090]
    type = PointValue
    variable = pp
    point = '90 0 0'
    execute_on = 'initial timestep_end'
  []
  [p100]
    type = PointValue
    variable = pp
    point = '100 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  print_linear_residuals = false
  csv = true
[]

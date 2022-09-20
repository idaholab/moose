# Intercomparison problem 3: Radial flow from an injection well
#
# From Pruess et al, Code intercomparison builds confidence in
# numerical simulation models for geologic disposal of CO2, Energy 29 (2004)
#
# A variation with zero salinity can be run by changing the initial condition
# of the AuxVariable xnacl

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 500
  xmax = 10000
  bias_x = 1.01
[]

[Problem]
  type = FEProblem
  coord_type = 'RZ'
  rz_coord_axis = Y
[]

[GlobalParams]
  PorousFlowDictator = 'dictator'
  gravity = '0 0 0'
[]

[AuxVariables]
  [pressure_liquid]
    order = CONSTANT
    family = MONOMIAL
  []
  [saturation_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1]
    order = CONSTANT
    family = MONOMIAL
  []
  [y0]
    order = CONSTANT
    family = MONOMIAL
  []
  [xnacl]
    initial_condition = 0.15
  []
[]

[AuxKernels]
  [pressure_liquid]
    type = PorousFlowPropertyAux
    variable = pressure_liquid
    property = pressure
    phase = 0
    execute_on = 'timestep_end'
  []
  [saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = 'timestep_end'
  []
  [x1]
    type = PorousFlowPropertyAux
    variable = x1
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = 'timestep_end'
  []
  [y0]
    type = PorousFlowPropertyAux
    variable = y0
    property = mass_fraction
    phase = 1
    fluid_component = 0
    execute_on = 'timestep_end'
  []
[]

[Variables]
  [pgas]
    initial_condition = 12e6
  []
  [zi]
    initial_condition = 0
    scaling = 1e4
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pgas
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pgas
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = zi
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = zi
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi'
    number_fluid_phases = 2
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 5.099e-5
    m = 0.457
    sat_lr = 0.0
    pc_max = 1e7
  []
  [fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2
    capillary_pressure = pc
  []
[]

[FluidProperties]
  [co2sw]
    type = CO2FluidProperties
  []
  [co2]
    type = TabulatedFluidProperties
    fp = co2sw
  []
  [water]
    type = Water97FluidProperties
  []
  [watertab]
    type = TabulatedFluidProperties
    fp = water
    temperature_min = 273.15
    temperature_max = 573.15
    fluid_property_file = water_fluid_properties.csv
    save_file = false
  []
  [brine]
    type = BrineFluidProperties
    water_fp = watertab
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = '45'
  []
  [brineco2]
    type = PorousFlowFluidState
    gas_porepressure = 'pgas'
    z = 'zi'
    temperature_unit = Celsius
    xnacl = 'xnacl'
    capillary_pressure = pc
    fluid_state = fs
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = '0.12'
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-13 0 0 0 1e-13 0 0 0 1e-13'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityVG
    m = 0.457
    phase = 0
    s_res = 0.3
    sum_s_res = 0.35
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
    s_res = 0.05
    sum_s_res = 0.35
  []
[]

[BCs]
  [rightwater]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'right'
    variable = pgas
    use_mobility = true
    PorousFlowDictator = dictator
    fluid_phase = 0
    multipliers = '0 1e9'
    PT_shift = '12e6'
    pt_vals = '0 1e9'
    mass_fraction_component = 0
    use_relperm = true
  []
  [rightco2]
    type = PorousFlowPiecewiseLinearSink
    variable = zi
    boundary = 'right'
    use_mobility = true
    PorousFlowDictator = dictator
    fluid_phase = 1
    multipliers = '0 1e9'
    PT_shift = '12e6'
    pt_vals = '0 1e9'
    mass_fraction_component = 1
    use_relperm = true
  []
[]

[DiracKernels]
  [source]
    type = PorousFlowSquarePulsePointSource
    point = '0 0 0'
    mass_flux = 1
    variable = zi
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres bjacobi lu NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 8.64e8
  nl_max_its = 25
  l_max_its = 100
  dtmax = 5e6
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 100
  []
[]

[VectorPostprocessors]
  [vars]
    type = NodalValueSampler
    sort_by = x
    variable = 'pgas zi xnacl'
    execute_on = 'timestep_end'
    outputs = spatial
  []
  [auxvars]
    type = ElementValueSampler
    sort_by = x
    variable = 'saturation_gas x1 y0'
    execute_on = 'timestep_end'
    outputs = spatial
  []
[]

[Postprocessors]
  [pgas]
    type = PointValue
    point = '25.25 0 0'
    variable = pgas
    outputs = time
  []
  [sgas]
    type = PointValue
    point = '25.25 0 0'
    variable = saturation_gas
    outputs = time
  []
  [zi]
    type = PointValue
    point = '25.25 0 0'
    variable = zi
    outputs = time
  []
  [massgas]
    type = PorousFlowFluidMass
    fluid_component = 1
    outputs = time
  []
  [x1]
    type = PointValue
    point = '25.25 0 0'
    variable = x1
    outputs = time
  []
  [y0]
    type = PointValue
    point = '25.25 0 0'
    variable = y0
    outputs = time
  []
  [xnacl]
    type = PointValue
    point = '25.25 0 0'
    variable = xnacl
    outputs = time
  []
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  sync_times = '2.592e6 8.64e6 8.64e7 8.64e8'
  [time]
    type = CSV
  []
  [spatial]
    type = CSV
    sync_only = true
  []
[]

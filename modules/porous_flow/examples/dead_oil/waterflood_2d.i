# Waterflood of a dead-oil reservoir with one injection well and one production well
#
# The reservoir is 200 m x 200 m x 10 m, initially at 20 MPa and containing oil at connate water
# saturation. Water is injected at a constant mass rate at (20, 20) and oil and water are
# produced from a well at (180, 180) with a fixed bottomhole pressure of 19.5 MPa.
#
# Phase 0 is water (SimpleFluidProperties) and phase 1 is dead oil, whose density and viscosity
# are interpolated from a PVDO-style table using TabulatedPVTFluidProperties. The fluid and rock
# properties are the same as in waterflood_1d.i.
#
# The production well is a PorousFlowPeacemanBorehole for each phase, so each phase is produced in
# proportion to its mobility. The injection well is a constant-rate point source: an injection
# PorousFlowPeacemanBorehole would inject nothing at first, because water is immobile at connate
# water saturation.
#
# Units: SI (Pa, s, kg, m)

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 40
  ny = 40
  nz = 1
  xmin = 0
  xmax = 200
  ymin = 0
  ymax = 200
  zmin = 0
  zmax = 10
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  # Water pressure: its kernels are the water (component 0) mass balance
  [pwater]
    initial_condition = 20e6
  []
  # Oil saturation: its kernels are the oil (component 1) mass balance
  [soil]
    initial_condition = 0.8 # connate water saturation of 0.2
  []
[]

[AuxVariables]
  # Water is entirely in phase 0 and oil is entirely in phase 1
  [frac_water_in_water]
    initial_condition = 1.0
  []
  [frac_water_in_oil]
    initial_condition = 0.0
  []
  [swater]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [swater]
    type = PorousFlowPropertyAux
    variable = swater
    property = saturation
    phase = 0
    execute_on = 'initial timestep_end'
  []
[]

[Kernels]
  [mass_water]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  []
  [flux_water]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  []
  [mass_oil]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = soil
  []
  [flux_oil]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = soil
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pwater soil'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
  # Total mass of each phase produced during each time step
  [water_produced]
    type = PorousFlowSumQuantity
  []
  [oil_produced]
    type = PorousFlowSumQuantity
  []
[]

[FluidProperties]
  [water]
    type = SimpleFluidProperties
    density0 = 1000
    bulk_modulus = 2.2e9
    viscosity = 0.5e-3
    thermal_expansion = 0
  []
  # Dead oil, PVDO-style table: pressure (Pa), formation volume factor (rm3/sm3), viscosity (Pa.s)
  [oil]
    type = TabulatedPVTFluidProperties
    density_sc = 850
    pressure = '1.0e7 2.0e7 3.0e7'
    formation_volume_factor = '1.050 1.038 1.028'
    viscosity = '1.10e-3 1.17e-3 1.24e-3'
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 350
  []
  [saturation_calculator]
    type = PorousFlow2PhasePS
    phase0_porepressure = pwater
    phase1_saturation = soil
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'frac_water_in_water frac_water_in_oil'
  []
  [water]
    type = PorousFlowSingleComponentFluid
    fp = water
    phase = 0
  []
  [oil]
    type = PorousFlowSingleComponentFluid
    fp = oil
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0  0 1e-12 0  0 0 1e-12'
  []
  # Corey relative permeabilities with connate water and residual oil saturations of 0.2
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.2
    sum_s_res = 0.4
  []
  [relperm_oil]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
    s_res = 0.2
    sum_s_res = 0.4
  []
[]

[DiracKernels]
  # Water injection at a constant mass rate (kg/s), at mid-thickness of the reservoir
  [injector]
    type = PorousFlowSquarePulsePointSource
    variable = pwater
    point = '20 20 5'
    mass_flux = 0.5
  []

  # Production well with radius 0.1 m (see producer.bh) and bottomhole pressure 19.5 MPa.
  # For PorousFlowPeacemanBorehole, use_mobility already includes the relative permeability
  [producer_water]
    type = PorousFlowPeacemanBorehole
    variable = pwater
    SumQuantityUO = water_produced
    point_file = producer.bh
    function_of = pressure
    fluid_phase = 0
    mass_fraction_component = 0
    bottom_p_or_t = 19.5e6
    unit_weight = '0 0 0'
    use_mobility = true # relative permeability x density / viscosity of the phase
    character = 1
  []
  [producer_oil]
    type = PorousFlowPeacemanBorehole
    variable = soil
    SumQuantityUO = oil_produced
    point_file = producer.bh
    function_of = pressure
    fluid_phase = 1
    mass_fraction_component = 1
    bottom_p_or_t = 19.5e6
    unit_weight = '0 0 0'
    use_mobility = true # relative permeability x density / viscosity of the phase
    character = 1
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
    outputs = none
  []
  [water_produced_this_step]
    type = PorousFlowPlotQuantity
    uo = water_produced
    outputs = none
  []
  [oil_produced_this_step]
    type = PorousFlowPlotQuantity
    uo = oil_produced
    outputs = none
  []
  # Production rates at surface conditions (m3/day)
  [oil_rate]
    type = ParsedPostprocessor
    expression = 'oil / dt / 850 * 86400'
    pp_names = 'oil_produced_this_step dt'
    pp_symbols = 'oil dt'
  []
  [water_rate]
    type = ParsedPostprocessor
    expression = 'water / dt / 1000 * 86400'
    pp_names = 'water_produced_this_step dt'
    pp_symbols = 'water dt'
  []
  [water_cut]
    type = ParsedPostprocessor
    expression = 'if(oil + water > 0, water / (oil + water), 0)'
    pp_names = 'oil_rate water_rate'
    pp_symbols = 'oil water'
  []
  # Cumulative production at surface conditions (m3)
  [cumulative_oil]
    type = CumulativeValuePostprocessor
    postprocessor = oil_volume_this_step
  []
  [oil_volume_this_step]
    type = ParsedPostprocessor
    expression = 'oil / 850'
    pp_names = 'oil_produced_this_step'
    pp_symbols = 'oil'
    outputs = none
  []
  [p_injector]
    type = PointValue
    variable = pwater
    point = '20 20 5'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 6.3072e7 # 2 years
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 20
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e4
    growth_factor = 1.5
    optimal_iterations = 6
  []
  dtmax = 1e6
[]

[Outputs]
  file_base = waterflood_2d
  csv = true
  [exodus]
    type = Exodus
    time_step_interval = 5
  []
[]

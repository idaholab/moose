# 1D waterflood of a dead-oil reservoir (Buckley-Leverett problem)
#
# Water is injected at a constant mass rate at the left end of a 100 m long domain that
# initially contains oil at connate water saturation. Oil and water are produced at the right
# end, where the pressure is held at 20 MPa through a free-outflow sink for each phase.
#
# Phase 0 is water (SimpleFluidProperties) and phase 1 is dead oil, whose density and viscosity
# are interpolated from a PVDO-style table using TabulatedPVTFluidProperties.
# Capillary pressure is zero, so the water saturation profile can be compared with the
# Buckley-Leverett solution computed by buckley_leverett.py
#
# Units: SI (Pa, s, kg, m)

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 200
  xmin = 0
  xmax = 100
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

[BCs]
  # Water injection at a constant mass flux (kg/m2/s). Negative means a source
  [water_injection]
    type = PorousFlowSink
    boundary = left
    variable = pwater
    flux_function = -1e-3
  []

  # Production: each phase flows out in proportion to its mobility and the difference between
  # its pressure and 20 MPa. With zero capillary pressure both phases have the same pressure
  [water_production]
    type = PorousFlowPiecewiseLinearSink
    boundary = right
    variable = pwater
    fluid_phase = 0
    mass_fraction_component = 0
    pt_vals = '0 1e9'
    multipliers = '0 1e9'
    PT_shift = 20e6
    use_mobility = true
    use_relperm = true
    flux_function = 10 # 1/L, with L the distance to the fixed-pressure production point
  []
  [oil_production]
    type = PorousFlowPiecewiseLinearSink
    boundary = right
    variable = soil
    fluid_phase = 1
    mass_fraction_component = 1
    pt_vals = '0 1e9'
    multipliers = '0 1e9'
    PT_shift = 20e6
    use_mobility = true
    use_relperm = true
    flux_function = 10
  []
[]

[Postprocessors]
  # Water saturation at a few points, for the regression test
  [sw_x10]
    type = PointValue
    variable = swater
    point = '10.1 0 0'
  []
  [sw_x20]
    type = PointValue
    variable = swater
    point = '20.1 0 0'
  []
  [sw_x30]
    type = PointValue
    variable = swater
    point = '30.1 0 0'
  []
  [sw_x40]
    type = PointValue
    variable = swater
    point = '40.1 0 0'
  []
  [sw_x50]
    type = PointValue
    variable = swater
    point = '50.1 0 0'
  []
  [p_inlet]
    type = PointValue
    variable = pwater
    point = '0 0 0'
  []
  [water_in_place]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [oil_in_place]
    type = PorousFlowFluidMass
    fluid_component = 1
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  [profile]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0.25 0 0'
    end_point = '99.75 0 0'
    num_points = 200
    sort_by = x
    variable = 'swater pwater'
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
  end_time = 4.32e6 # 50 days
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
  nl_max_its = 20
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e3
    growth_factor = 1.5
    optimal_iterations = 6
  []
  dtmax = 2e4
[]

[Outputs]
  file_base = waterflood_1d
  [csv]
    type = CSV
    sync_only = true
    sync_times = '2.16e6 4.32e6' # 25 and 50 days
  []
[]

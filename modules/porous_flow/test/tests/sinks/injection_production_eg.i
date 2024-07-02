# phase = 0 is liquid phase
# phase = 1 is gas phase
# fluid_component = 0 is water
# fluid_component = 1 is CO2

# Constant rate of CO2 injection into the left boundary
# 1D mesh
# The PorousFlowPiecewiseLinearSinks remove the correct water and CO2 from the right boundary

# Note i take pretty big timesteps here so the system is quite nonlinear

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 20
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[AuxVariables]
  [saturation_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [frac_water_in_liquid]
    initial_condition = 1.0
  []
  [frac_water_in_gas]
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = timestep_end
  []
[]

[Variables]
  [pwater]
    initial_condition = 20E6
  []
  [pgas]
    initial_condition = 20.1E6
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pgas
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = pgas
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas pwater'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1E-6
    m = 0.6
  []
[]

[FluidProperties]
  [true_water]
    type = Water97FluidProperties
  []
  [tabulated_water]
    type = TabulatedBicubicFluidProperties
    fp = true_water
    temperature_min = 275
    pressure_max = 1E8
    interpolated_properties = 'density viscosity enthalpy internal_energy'
    fluid_property_output_file = water97_tabulated_11.csv
    # Comment out the fp parameter and uncomment below to use the newly generated tabulation
    # fluid_property_file = water97_tabulated_11.csv
  []
  [true_co2]
    type = CO2FluidProperties
  []
  [tabulated_co2]
    type = TabulatedBicubicFluidProperties
    fp = true_co2
    temperature_min = 275
    pressure_max = 1E8
    interpolated_properties = 'density viscosity enthalpy internal_energy'
    fluid_property_output_file = co2_tabulated_11.csv
    # Comment out the fp parameter and uncomment below to use the newly generated tabulation
    # fluid_property_file = co2_tabulated_11.csv
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 293.15
  []
  [saturation_calculator]
    type = PorousFlow2PhasePP
    phase0_porepressure = pwater
    phase1_porepressure = pgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'frac_water_in_liquid frac_water_in_gas'
  []
  [water]
    type = PorousFlowSingleComponentFluid
    fp = tabulated_water
    phase = 0
  []
  [co2]
    type = PorousFlowSingleComponentFluid
    fp = tabulated_co2
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.1
    sum_s_res = 0.2
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityBC
    nw_phase = true
    lambda = 2
    s_res = 0.1
    sum_s_res = 0.2
    phase = 1
  []
[]

[BCs]
  [co2_injection]
    type = PorousFlowSink
    boundary = left
    variable = pgas # pgas is associated with the CO2 mass balance (fluid_component = 1 in its Kernels)
    flux_function = -1E-2 # negative means a source, rather than a sink
  []

  [right_water]
    type = PorousFlowPiecewiseLinearSink
    boundary = right

    # a sink of water, since the Kernels given to pwater are for fluid_component = 0 (the water)
    variable = pwater

    # this Sink is a function of liquid porepressure
    # Also, all the mass_fraction, mobility and relperm are referenced to the liquid phase now
    fluid_phase = 0

    # Sink strength = (Pwater - 20E6)
    pt_vals = '0 1E9'
    multipliers = '0 1E9'
    PT_shift = 20E6

    # multiply Sink strength computed above by mass fraction of water at the boundary
    mass_fraction_component = 0

    # also multiply Sink strength by mobility of the liquid
    use_mobility = true

    # also multiply Sink strength by the relperm of the liquid
    use_relperm = true

    # also multiplly Sink strength by 1/L, where L is the distance to the fixed-porepressure external environment
    flux_function = 10 # 1/L
  []

  [right_co2]
    type = PorousFlowPiecewiseLinearSink
    boundary = right
    variable = pgas
    fluid_phase = 1
    pt_vals = '0 1E9'
    multipliers = '0 1E9'
    PT_shift = 20.1E6
    mass_fraction_component = 1
    use_mobility = true
    use_relperm = true
    flux_function = 10 # 1/L
  []
[]

[Preconditioning]
  active = 'basic'
  [basic]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu NONZERO 2'
  []
  [preferred]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = 'lu mumps'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_abs_tol = 1E-13
  nl_rel_tol = 1E-10
  end_time = 1e4
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1E4
    growth_factor = 1.1
  []
[]

[VectorPostprocessors]
  [pps]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0 0 0'
    end_point = '20 0 0'
    num_points = 20
    sort_by = x
    variable = 'pgas pwater saturation_gas'
  []
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  [out]
    type = CSV
    execute_on = final
  []
[]

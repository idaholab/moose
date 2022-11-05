# Two-phase borehole injection problem in RZ coordinates
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmin = 1.0
    xmax = 10
    bias_x = 1.4
    ny = 3
    ymin = -6
    ymax = 6
  []
  [aquifer]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 -2 0'
    top_right = '10 2 0'
  []
  [injection_area]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x<1.0001'
    included_subdomain_ids = 1
    new_sideset_name = 'injection_area'
    input = 'aquifer'
  []
  [rename]
    type = RenameBlockGenerator
    old_block = '0 1'
    new_block = 'caps aquifer'
    input = 'injection_area'
  []
[]

[Problem]
  coord_type = RZ
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pwater pgas T disp_r'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1E-6
    m = 0.6
  []
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  gravity = '0 0 0'
  biot_coefficient = 1.0
  PorousFlowDictator = dictator
[]

[Variables]
  [pwater]
    initial_condition = 20E6
  []
  [pgas]
    initial_condition = 20.1E6
  []
  [T]
    initial_condition = 330
    scaling = 1E-5
  []
  [disp_r]
    scaling = 1E-5
  []
[]

[Kernels]
  [mass_water_dot]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  []
  [flux_water]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    use_displaced_mesh = false
    variable = pwater
  []
  [vol_strain_rate_water]
    type = PorousFlowMassVolumetricExpansion
    fluid_component = 0
    variable = pwater
  []
  [mass_co2_dot]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pgas
  []
  [flux_co2]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    use_displaced_mesh = false
    variable = pgas
  []
  [vol_strain_rate_co2]
    type = PorousFlowMassVolumetricExpansion
    fluid_component = 1
    variable = pgas
  []
  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = T
  []
  [advection]
    type = PorousFlowHeatAdvection
    use_displaced_mesh = false
    variable = T
  []
  [conduction]
    type = PorousFlowHeatConduction
    use_displaced_mesh = false
    variable = T
  []
  [vol_strain_rate_heat]
    type = PorousFlowHeatVolumetricExpansion
    variable = T
  []
  [grad_stress_r]
    type = StressDivergenceRZTensors
    temperature = T
    variable = disp_r
    eigenstrain_names = thermal_contribution
    use_displaced_mesh = false
    component = 0
  []
  [poro_r]
    type = PorousFlowEffectiveStressCoupling
    variable = disp_r
    use_displaced_mesh = false
    component = 0
  []
[]

[AuxVariables]
  [disp_z]
  []
  [effective_fluid_pressure]
    family = MONOMIAL
    order = CONSTANT
  []
  [mass_frac_phase0_species0]
    initial_condition = 1 # all water in phase=0
  []
  [mass_frac_phase1_species0]
    initial_condition = 0 # no water in phase=1
  []
  [sgas]
    family = MONOMIAL
    order = CONSTANT
  []
  [swater]
    family = MONOMIAL
    order = CONSTANT
  []
  [stress_rr]
    family = MONOMIAL
    order = CONSTANT
  []
  [stress_tt]
    family = MONOMIAL
    order = CONSTANT
  []
  [stress_zz]
    family = MONOMIAL
    order = CONSTANT
  []
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [effective_fluid_pressure]
    type = ParsedAux
    coupled_variables = 'pwater pgas swater sgas'
    expression = 'pwater * swater + pgas * sgas'
    variable = effective_fluid_pressure
  []
  [swater]
    type = PorousFlowPropertyAux
    variable = swater
    property = saturation
    phase = 0
    execute_on = timestep_end
  []
  [sgas]
    type = PorousFlowPropertyAux
    variable = sgas
    property = saturation
    phase = 1
    execute_on = timestep_end
  []
  [stress_rr_aux]
    type = RankTwoAux
    variable = stress_rr
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  []
  [stress_tt]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_tt
    index_i = 2
    index_j = 2
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 1
    index_j = 1
  []
  [porosity]
    type = PorousFlowPropertyAux
    variable = porosity
    property = porosity
    execute_on = timestep_end
  []
[]


[BCs]
  [pinned_top_bottom_r]
    type = DirichletBC
    variable = disp_r
    value = 0
    boundary = 'top bottom'
  []
  [cavity_pressure_r]
    type = Pressure
    boundary = injection_area
    variable = disp_r
    postprocessor = constrained_effective_fluid_pressure_at_wellbore
    use_displaced_mesh = false
  []

  [cold_co2]
    type = DirichletBC
    boundary = injection_area
    variable = T
    value = 290  # injection temperature
    use_displaced_mesh = false
  []
  [constant_co2_injection]
    type = PorousFlowSink
    boundary = injection_area
    variable = pgas
    fluid_phase = 1
    flux_function = -1E-4
    use_displaced_mesh = false
  []

  [outer_water_removal]
    type = PorousFlowPiecewiseLinearSink
    boundary = right
    variable = pwater
    fluid_phase = 0
    pt_vals = '0 1E9'
    multipliers = '0 1E8'
    PT_shift = 20E6
    use_mobility = true
    use_relperm = true
    use_displaced_mesh = false
  []
  [outer_co2_removal]
    type = PorousFlowPiecewiseLinearSink
    boundary = right
    variable = pgas
    fluid_phase = 1
    pt_vals = '0 1E9'
    multipliers = '0 1E8'
    PT_shift = 20.1E6
    use_mobility = true
    use_relperm = true
    use_displaced_mesh = false
  []
[]

[FluidProperties]
  [true_water]
    type = Water97FluidProperties
  []
  [tabulated_water]
    type = TabulatedFluidProperties
    fp = true_water
    temperature_min = 275
    pressure_max = 1E8
    fluid_property_file = water97_tabulated_11.csv
  []
  [true_co2]
    type = CO2FluidProperties
  []
  [tabulated_co2]
    type = TabulatedFluidProperties
    fp = true_co2
    temperature_min = 275
    pressure_max = 1E8
    fluid_property_file = co2_tabulated_11.csv
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = T
  []
  [saturation_calculator]
    type = PorousFlow2PhasePP
    phase0_porepressure = pwater
    phase1_porepressure = pgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'mass_frac_phase0_species0 mass_frac_phase1_species0'
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
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 4
    s_res = 0.1
    sum_s_res = 0.2
    phase = 0
  []
  [relperm_co2]
    type = PorousFlowRelativePermeabilityBC
    nw_phase = true
    lambda = 2
    s_res = 0.1
    sum_s_res = 0.2
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosity
    fluid = true
    mechanical = true
    thermal = true
    porosity_zero = 0.1
    reference_temperature = 330
    reference_porepressure = 20E6
    thermal_expansion_coeff = 15E-6 # volumetric
    solid_bulk = 8E9 # unimportant since biot = 1
  []
  [permeability_aquifer]
    type = PorousFlowPermeabilityKozenyCarman
    block = aquifer
    poroperm_function = kozeny_carman_phi0
    phi0 = 0.1
    n = 2
    m = 2
    k0 = 1E-12
  []
  [permeability_caps]
    type = PorousFlowPermeabilityKozenyCarman
    block = caps
    poroperm_function = kozeny_carman_phi0
    phi0 = 0.1
    n = 2
    m = 2
    k0 = 1E-15
    k_anisotropy = '1 0 0  0 1 0  0 0 0.1'
  []
  [rock_thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '2 0 0  0 2 0  0 0 2'
  []
  [rock_internal_energy]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1100
    density = 2300
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 5E9
    poissons_ratio = 0.0
  []
  [strain]
    type = ComputeAxisymmetricRZSmallStrain
    eigenstrain_names = 'thermal_contribution initial_stress'
  []
  [thermal_contribution]
    type = ComputeThermalExpansionEigenstrain
    temperature = T
    thermal_expansion_coeff = 5E-6 # this is the linear thermal expansion coefficient
    eigenstrain_name = thermal_contribution
    stress_free_temperature = 330
  []
  [initial_strain]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '20E6 0 0  0 20E6 0  0 0 20E6'
    eigenstrain_name = initial_stress
  []
  [stress]
    type = ComputeLinearElasticStress
  []

  [effective_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [volumetric_strain]
    type = PorousFlowVolumetricStrain
  []
[]

[Postprocessors]
  [effective_fluid_pressure_at_wellbore]
    type = PointValue
    variable = effective_fluid_pressure
    point = '1 0 0'
    execute_on = timestep_begin
    use_displaced_mesh = false
  []
  [constrained_effective_fluid_pressure_at_wellbore]
    type = FunctionValuePostprocessor
    function = constrain_effective_fluid_pressure
    execute_on = timestep_begin
  []
[]

[Functions]
  [constrain_effective_fluid_pressure]
    type = ParsedFunction
    symbol_names = effective_fluid_pressure_at_wellbore
    symbol_values = effective_fluid_pressure_at_wellbore
    expression = 'max(effective_fluid_pressure_at_wellbore, 20E6)'
  []
[]

[Preconditioning]
  active = basic
  [basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
  [preferred_but_might_not_be_installed]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E3
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1E3
    growth_factor = 1.2
    optimal_iterations = 10
  []
  nl_abs_tol = 1E-7
[]

[Outputs]
  exodus = true
[]

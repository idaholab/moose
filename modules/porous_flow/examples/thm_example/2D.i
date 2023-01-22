# Two phase, temperature-dependent, with mechanics, radial with fine mesh, constant injection of cold co2 into a overburden-reservoir-underburden containing mostly water
# species=0 is water
# species=1 is co2
# phase=0 is liquid, and since massfrac_ph0_sp0 = 1, this is all water
# phase=1 is gas, and since massfrac_ph1_sp0 = 0, this is all co2
#
# The mesh used below has very high resolution, so the simulation takes a long time to complete.
# Some suggested meshes of different resolution:
# nx=50, bias_x=1.2
# nx=100, bias_x=1.1
# nx=200, bias_x=1.05
# nx=400, bias_x=1.02
# nx=1000, bias_x=1.01
# nx=2000, bias_x=1.003
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2000
  bias_x = 1.003
  xmin = 0.1
  xmax = 5000
  ny = 1
  ymin = 0
  ymax = 11
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  PorousFlowDictator = dictator
  gravity = '0 0 0'
  biot_coefficient = 1.0
[]

[Variables]
  [pwater]
    initial_condition = 18.3e6
  []
  [sgas]
    initial_condition = 0.0
  []
  [temp]
    initial_condition = 358
  []
  [disp_r]
  []
[]

[AuxVariables]
  [rate]
  []
  [disp_z]
  []
  [massfrac_ph0_sp0]
    initial_condition = 1 # all H20 in phase=0
  []
  [massfrac_ph1_sp0]
    initial_condition = 0 # no H2O in phase=1
  []
  [pgas]
    family = MONOMIAL
    order = FIRST
  []
  [swater]
    family = MONOMIAL
    order = FIRST
  []
  [stress_rr]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_tt]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
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
  [mass_co2_dot]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux_co2]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    use_displaced_mesh = false
    variable = sgas
  []
  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  []
  [advection]
    type = PorousFlowHeatAdvection
    use_displaced_mesh = false
    variable = temp
  []
  [conduction]
    type = PorousFlowExponentialDecay
    use_displaced_mesh = false
    variable = temp
    reference = 358
    rate = rate
  []
  [grad_stress_r]
    type = StressDivergenceRZTensors
    temperature = temp
    eigenstrain_names = thermal_contribution
    variable = disp_r
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

[AuxKernels]
  [rate]
    type = FunctionAux
    variable = rate
    execute_on = timestep_begin
    function = decay_rate
  []
  [pgas]
    type = PorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = pgas
  []
  [swater]
    type = PorousFlowPropertyAux
    property = saturation
    phase = 0
    variable = swater
  []
  [stress_rr]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_rr
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
[]

[Functions]
  [decay_rate]
# Eqn(26) of the first paper of LaForce et al.
# Ka * (rho C)_a = 10056886.914
# h = 11
    type = ParsedFunction
    expression = 'sqrt(10056886.914/t)/11.0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pwater sgas disp_r'
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
    bulk_modulus = 2.27e14
    density0 = 970.0
    viscosity = 0.3394e-3
    cv = 4149.0
    cp = 4149.0
    porepressure_coefficient = 0.0
    thermal_expansion = 0
  []
  [co2]
    type = SimpleFluidProperties
    bulk_modulus = 2.27e14
    density0 = 516.48
    viscosity = 0.0393e-3
    cv = 2920.5
    cp = 2920.5
    porepressure_coefficient = 0.0
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = pwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [water]
    type = PorousFlowSingleComponentFluid
    fp = water
    phase = 0
  []
  [gas]
    type = PorousFlowSingleComponentFluid
    fp = co2
    phase = 1
  []
  [porosity_reservoir]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [permeability_reservoir]
    type = PorousFlowPermeabilityConst
    permeability = '2e-12 0 0  0 0 0  0 0 0'
  []
  [relperm_liquid]
    type = PorousFlowRelativePermeabilityCorey
    n = 4
    phase = 0
    s_res = 0.200
    sum_s_res = 0.405
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityBC
    phase = 1
    s_res = 0.205
    sum_s_res = 0.405
    nw_phase = true
    lambda = 2
  []
  [thermal_conductivity_reservoir]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0 0 0  0 1.320 0  0 0 0'
    wet_thermal_conductivity = '0 0 0  0 3.083 0  0 0 0'
  []
  [internal_energy_reservoir]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1100
    density = 2350.0
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    shear_modulus = 6.0E9
    poissons_ratio = 0.2
  []
  [strain]
    type = ComputeAxisymmetricRZSmallStrain
    eigenstrain_names = 'thermal_contribution ini_stress'
  []
  [ini_strain]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '-12.8E6 0 0  0 -51.3E6 0  0 0 -12.8E6'
    eigenstrain_name = ini_stress
  []
  [thermal_contribution]
    type = ComputeThermalExpansionEigenstrain
    temperature = temp
    stress_free_temperature = 358
    thermal_expansion_coeff = 5E-6
    eigenstrain_name = thermal_contribution
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [eff_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [vol_strain]
    type = PorousFlowVolumetricStrain
  []
[]

[BCs]
  [outer_pressure_fixed]
    type = DirichletBC
    boundary = right
    value = 18.3e6
    variable = pwater
  []
  [outer_saturation_fixed]
    type = DirichletBC
    boundary = right
    value = 0.0
    variable = sgas
  []
  [outer_temp_fixed]
    type = DirichletBC
    boundary = right
    value = 358
    variable = temp
  []
  [fixed_outer_r]
    type = DirichletBC
    variable = disp_r
    value = 0
    boundary = right
  []
  [co2_injection]
    type = PorousFlowSink
    boundary = left
    variable = sgas
    use_mobility = false
    use_relperm = false
    fluid_phase = 1
    flux_function = 'min(t/100.0,1)*(-2.294001475)' # 5.0E5 T/year = 15.855 kg/s, over area of 2Pi*0.1*11
  []
  [cold_co2]
    type = DirichletBC
    boundary = left
    variable = temp
    value = 294
  []
  [cavity_pressure_x]
    type = Pressure
    boundary = left
    variable = disp_r
    component = 0
    postprocessor = p_bh # note, this lags
    use_displaced_mesh = false
  []
[]

[Postprocessors]
  [p_bh]
    type = PointValue
    variable = pwater
    point = '0.1 0 0'
    execute_on = timestep_begin
    use_displaced_mesh = false
  []
[]

[VectorPostprocessors]
  [ptsuss]
    type = LineValueSampler
    use_displaced_mesh = false
    start_point = '0.1 0 0'
    end_point = '5000 0 0'
    sort_by = x
    num_points = 50000
    outputs = csv
    variable = 'pwater temp sgas disp_r stress_rr stress_tt'
  []
[]

[Preconditioning]
  active = 'smp'
  [smp]
    type = SMP
    full = true
    #petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2               1E2       1E-5        500'
  []
  [mumps]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt -snes_linesearch_monitor'
    petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package -pc_factor_shift_type -snes_rtol -snes_atol -snes_max_it'
    petsc_options_value = 'gmres      lu       mumps                         NONZERO               1E-5       1E2       50'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 1.5768e8
  #dtmax = 1e6
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    growth_factor = 1.1
  []
[]

[Outputs]
  print_linear_residuals = false
  sync_times = '3600 86400 2.592E6 1.5768E8'
  perf_graph = true
  exodus = true
  [csv]
    type = CSV
    sync_only = true
  []
[]

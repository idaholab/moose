[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = false
[]

[Mesh]
  coord_type = XYZ
  [fmesh]
    type = FileMeshGenerator
    file = 'mesh_in.e'
  []
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = ref
  extra_tag_vectors = ref
  group_variables = 'disp_x disp_y disp_z'
[]

[Functions]
  [fission_density_fcn] # fis/m3
    type = PiecewiseLinear
    data_file = 'average_fission_density.csv'
    scale_factor = 1e27
    format = columns
  []
  [fluence_fcn] # n/m2
    type = PiecewiseLinear
    data_file = 'FLUENCE.csv'
    scale_factor = 1
    format = columns
  []
  [power_density]
    type = PiecewiseLinear
    data_file = 'POWER_DENSITY.csv' # W/cm3 to W/m3
    scale_factor = 1e6
    format = columns
  []
  [L2AR]
    type = ParsedFunction
    # fissprof_l2ar is the distribution from the L2AR tab: L2AR_ABAQUS at cell R64
    vars = 'x_offset y_offset'
    vals = '9.4615e-3 3.175e-3'
    value = 'x0 := x-x_offset;
             y0 := y-y_offset;
             (4.669e-10*x0^5 - 8.657e-8*x0^4 + 5.877e-6*x0^3 - 0.0001962*x0^2 + 0.004373*x0 + 0.9446) * (5.047e-7*y0^6 - 2.974e-05*y0^5 + 0.0006881*y0^4 - 0.007883*y0^3 + 0.04726*y0^2 - 0.1458*y0 + 1.152)'
  []
  [power_history]
    type = CompositeFunction
    functions = 'power_density L2AR'
  []
[]

[Variables]
  [temperature]
    initial_condition = 294
  []
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temperature
  []
  [heat_ie]
    type = HeatConductionTimeDerivative
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    block = fuel
    function = power_history
    variable = temperature
    extra_vector_tags = 'ref'
  []
[]

[Modules/TensorMechanics/Master]
  add_variables = true
  strain = FINITE
  temperature = temperature
  generate_output = 'stress_xx stress_yy stress_zz vonmises_stress
                     hydrostatic_stress elastic_strain_xx elastic_strain_yy
                     elastic_strain_zz strain_xx strain_yy strain_zz'
  [fuel_mechanics]
    block = fuel
    eigenstrain_names = 'fuel_thermal_strain'
    extra_vector_tags = 'ref'
  []
  [clad_mechanics]
    block = clad
    eigenstrain_names = 'clad_thermal_strain'
    extra_vector_tags = ref
  []
[]

[BCs]
  [conv_BC_front]
    type = ConvectiveHeatFluxBC
    variable = temperature
    boundary = 'clad_wall'
    T_infinity = 'tfilm_outer' # film temperature
    heat_transfer_coefficient = 'htc_outer'
    heat_transfer_coefficient_dT = 'dhtc_dT_outer'
  []
  [conv_BC_back]
    type = ConvectiveHeatFluxBC
    variable = temperature
    boundary = 'clad_wall'
    T_infinity = 'tfilm_inner' # film temperature
    heat_transfer_coefficient = 'htc_inner'
    heat_transfer_coefficient_dT = 'dhtc_dT_inner'
  []
  [disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_to_clad top_to_clad'
    value = 0.0
  []
  [disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom_to_clad top_to_clad'
    value = 0.0
  []
  [disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom_to_clad top_to_clad'
    value = 0.0
  []
[]

[Materials]
  [fission_density]
    type = GenericFunctionMaterial
    prop_names = fission_density
    prop_values = fission_density_fcn
  []
  [fluence]
    type = GenericFunctionMaterial
    prop_names = fast_neutron_fluence
    prop_values = fluence_fcn
    outputs = all
  []
  [tfilm_inner]
    type = ParsedMaterial
    f_name = tfilm_inner
    args = temperature
    function = '325.15'
  []
  [htc_inner]
    type = ParsedMaterial
    f_name = htc_inner
    args = 'temperature'
    function = '-3.3647*temperature^2 + 2744.5*temperature - 454016.0'
  []
  [dhtc_dT_inner]
    type = ParsedMaterial
    f_name = dhtc_dT_inner
    args = 'temperature'
    function = '-6.7294*temperature + 2744.5'
  []
  [tfilm_outer]
    type = ParsedMaterial
    f_name = tfilm_outer
    args = temperature
    function = '325.15'
  []
  [htc_outer]
    type = ParsedMaterial
    f_name = htc_outer
    args = 'temperature'
    function = '-2.8992*temperature^2 + 2354.8*temperature - 387945.0'
  []
  [dhtc_dT_outer]
    type = ParsedMaterial
    f_name = dhtc_dT_outer
    args = 'temperature'
    function = '-5.7984*temperature + 2354.8'
  []
  # fuel properties
  [fuel_thermal]
    type = U10MoThermal
    block = fuel
    temperature = temperature
    thermal_conductivity_model = USHPRR
    specific_heat_model = USHPRR
  []
  [fuel_elasticity]
    type = U10MoElasticityTensor
    block = fuel
    temperature = temperature
  []
  [fuel_thermal_expansion]
    type = U10MoThermalExpansionEigenstrain
    block = fuel
    temperature = temperature
    eigenstrain_name = fuel_thermal_strain
    stress_free_temperature = 298
    model_option = Rest
  []
  [fuel_stress]
    type = ComputeFiniteStrainElasticStress
    block = fuel
  []
  #[fuel_swelling]
  #  type = U10MoVolumetricSwellingEigenstrain
  #  eigenstrain_name = fuel_swelling_strain
  #  fission_rate = 0
  #[]
  [fuel_density]
    type = ParsedMaterial
    block = fuel
    f_name = density
    args = 'temperature'
    function = '-0.884*temperature + 17391.0'
    outputs = exodus
  []
  # clad properties
  [clad_thermal]
    type = Al6061Thermal
    temperature = temperature
    block = clad
    thermal_conductivity_model = T6
    specific_heat_model = ASM
  []
  [clad_elasticity]
    type = Al6061ElasticityTensor
    block = clad
    temperature = temperature
    fast_neutron_fluence = 0
    youngs_modulus_model = Kaufman
  []
  [clad_thermal_expansion]
    type = Al6061ThermalExpansionEigenstrain
    stress_free_temperature = 293.15
    temperature = temperature
    eigenstrain_name = clad_thermal_strain
    block = clad
  []
  [clad_stress]
    type = ComputeFiniteStrainElasticStress
    block = clad
  []
  [clad_density]
    type = ParsedMaterial
    block = clad
    f_name = density
    args = 'temperature'
    function = '-0.0003*temperature^2 + 0.0966*temperature + 2690.4'
    outputs = exodus
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
  solve_type = 'Newton'

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'
  #petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  #petsc_options_value = 'lu superlu_dist'

  line_search = 'none'

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  nl_max_its = 50

  l_tol = 1e-4
  l_max_its = 50

  start_time = 0.0
  end_time = 4586400
  num_steps= 2500

  dtmax = 86400

  [Predictor]
    type = SimplePredictor
    scale = 1
  []

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1800
    optimal_iterations = 12
    iteration_window = 2
    growth_factor = 2
    cutback_factor = .5
    timestep_limiting_function = power_density
    force_step_every_function_point = true
  []
[]

[Postprocessors]
  [average_fuel_T]
    type = ElementAverageValue
    block = 'fuel'
    variable = temperature
    execute_on = 'initial timestep_end'
  []
  [max_fuel_T]
    type = ElementExtremeValue
    block = 'fuel'
    value_type = max
    variable = temperature
    execute_on = 'initial timestep_end'
  []
  [peacking_factor]
    type = ParsedPostprocessor
    pp_names = 'max_fuel_T average_fuel_T'
    function = max_fuel_T/average_fuel_T
  []
  #[fuelVolume]
  #  type = InternalVolume
  #  boundary = fuel_all
  #[]
  #[totalVolume]
  #  type = InternalVolume
  #  boundary = all
  #[]
  [power_history]
    type = FunctionValuePostprocessor
    function = power_history
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
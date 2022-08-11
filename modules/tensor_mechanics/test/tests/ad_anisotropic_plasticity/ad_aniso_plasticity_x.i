[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 2
    nz = 2
    xmin = 0.0
    ymin = 0.0
    zmin = 0.0

    xmax = 10.0
    ymax = 1.0
    zmax = 1.0
  []
  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = '100'
    nodes = '3 69'
    input = gen
  []
  [corner_node_2]
    type = ExtraNodesetGenerator
    new_boundary = '101'
    nodes = '4 47'
    input = corner_node
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[AuxVariables]
  [hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [hydrostatic_stress]
    type = ADRankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  []
  [plasticity_strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = trial_plasticity_plastic_strain
    variable = plastic_strain_xx
    index_i = 0
    index_j = 0
  []
  [plasticity_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = trial_plasticity_plastic_strain
    variable = plastic_strain_xy
    index_i = 0
    index_j = 1
  []
  [plasticity_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = trial_plasticity_plastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
  []
  [plasticity_strain_zz]
    type = ADRankTwoAux
    rank_two_tensor = trial_plasticity_plastic_strain
    variable = plastic_strain_zz
    index_i = 2
    index_j = 2
  []
  [sigma_xx]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 1
    index_j = 1
  []
  [elastic_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_yy
    index_i = 1
    index_j = 1
  []
  [sigma_yy]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 1e1 1e8'
    y = '0 -4e2 -4e2'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    generate_output = 'elastic_strain_xx stress_xx'
    use_automatic_differentiation = true
    add_variables = true
  []
[]

[Materials]

  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 70000
    poissons_ratio = 0.25
  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_plasticity"
    max_iterations = 50
    absolute_tolerance = 1e-16
  []

  [hill_tensor]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.5829856 0.364424 0.6342174 2.0691375 2.3492325 1.814589"
    base_name = trial_plasticity
  []

  [trial_plasticity]
    type = ADHillPlasticityStressUpdate
    hardening_constant = 2000.0
    yield_stress = 0.001 # was 200 for verification
    absolute_tolerance = 1e-14
    relative_tolerance = 1e-12
    base_name = trial_plasticity
    internal_solve_full_iteration_history = true
    max_inelastic_increment = 2.0e-6
    internal_solve_output_on = on_error
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  [no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  []

  [no_disp_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = 101
    value = 0.0
  []

  [Pressure]
    [Side1]
      boundary = right
      function = pull
    []
  []

[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  nl_rel_tol = 1e-11
  nl_abs_tol = 1.0e-14
  l_max_its = 90
  num_steps = 25
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 30
    iteration_window = 9
    growth_factor = 1.05
    cutback_factor = 0.5
    timestep_limiting_postprocessor = matl_ts_min
    dt = 1.0e-5
    time_t = '0 3.4e-5 10'
    time_dt = '1.0e-5 1.0e-7 1.0e-7'
  []
  start_time = 0
  automatic_scaling = true
[]

[Postprocessors]
  [matl_ts_min]
    type = MaterialTimeStepPostprocessor
  []
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
  []
  [max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
  []
  [max_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  []
  [dt]
    type = TimestepSize
  []
  [num_lin]
    type = NumLinearIterations
    outputs = console
  []
  [num_nonlin]
    type = NumNonlinearIterations
    outputs = console
  []
  [plasticity_strain_yy]
    type = ElementalVariableValue
    variable = plastic_strain_yy
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [elastic_strain_yy]
    type = ElementalVariableValue
    variable = elastic_strain_yy
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [sigma_yy]
    type = ElementalVariableValue
    variable = stress_yy
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [plasticity_strain_xx]
    type = ElementalVariableValue
    variable = plastic_strain_xx
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [elastic_strain_xx]
    type = ElementalVariableValue
    variable = elastic_strain_xx
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [sigma_xx]
    type = ElementalVariableValue
    variable = stress_xx
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [plasticity_strain_zz]
    type = ElementalVariableValue
    variable = plastic_strain_zz
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
[]

[Outputs]
  csv = true
  perf_graph = true
[]

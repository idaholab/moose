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
  [creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [hydrostatic_stress]
    type = RankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  []
  [creep_strain_xx]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xx
    index_i = 0
    index_j = 0
  []
  [creep_strain_xy]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xy
    index_i = 0
    index_j = 1
  []
  [creep_strain_yy]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_yy
    index_i = 1
    index_j = 1
  []
  [sigma_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 1
    index_j = 1
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 1.0'
    y = '0 -4e1'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    generate_output = 'elastic_strain_xx stress_xx'
    use_automatic_differentiation = false
    add_variables = true
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 7000
    poissons_ratio = 0.0
  []

  [elastic_strain]
    type = ComputeMultipleInelasticStress
    inelastic_models = "trial_creep_two"
    max_iterations = 50
    absolute_tolerance = 1e-16
  []

  [hill_tensor]
    type = HillConstants
    # F G H L M N
    hill_constants = "0.5 0.25 0.3866 1.6413 1.6413 1.2731"
  []

  [trial_creep_two]
    type = HillCreepStressUpdate
    coefficient = 1e-16
    n_exponent = 9
    m_exponent = 0
    activation_energy = 0
    max_inelastic_increment = 1.0e-4
    absolute_tolerance = 1e-20
    relative_tolerance = 1e-20
    max_integration_error = 1.0e-5
  []
[]

[BCs]
  [no_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []

  [no_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  []

  [no_disp_z]
    type = DirichletBC
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

[UserObjects]
  [terminator_creep]
    type = Terminator
    expression = 'time_step_size > matl_ts_min'
    fail_mode = SOFT
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  nl_rel_tol = 1.0e-13
  nl_abs_tol = 1.0e-13
  l_max_its = 10
  end_time = 1.65e-1
  dt = 2.5e-2
  start_time = 0
  automatic_scaling = true

  [./TimeStepper]
  type = IterationAdaptiveDT
  dt = 2.5e-2
  time_t = '0.0  10.0'
  time_dt = '2.5e-2 2.5e-2 '
  optimal_iterations = 30
  iteration_window = 9
  growth_factor = 1.5
  cutback_factor = 0.5
  timestep_limiting_postprocessor = matl_ts_min
[../]
[]

[Postprocessors]
  [time_step_size]
    type = TimestepSize
  []
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
  [creep_strain_xx]
    type = ElementalVariableValue
    variable = creep_strain_xx
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
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]

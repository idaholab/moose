[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  second_order = true
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'elastic_strain_xx elastic_strain_yy elastic_strain_xy stress_xx stress_xy '
                      'stress_yy'
    use_automatic_differentiation = true
  []
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

[Variables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
[]

[AuxKernels]
  [hydrostatic_stress]
    type = ADRankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  []
  [creep_strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = trial_creep_creep_strain
    variable = creep_strain_xx
    index_i = 0
    index_j = 0
  []
  [creep_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = trial_creep_creep_strain
    variable = creep_strain_xy
    index_i = 0
    index_j = 1
  []
  [creep_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = trial_creep_creep_strain
    variable = creep_strain_yy
    index_i = 1
    index_j = 1
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 1e3 1e8'
    y = '0 1e2 1e2'
  []
[]

[Materials]
  # Supplying elasticity tensor three times with different base_name
  [elasticity_tensor_three]
    type = ADComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '2.0e3 2.0e5 2.0e3 0.71428571e3 0.71428571e3 0.71428571e3 0.4 0.2 0.004 0.004 0.2 0.4'
  []
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '2.0e3 2.0e5 2.0e3 0.71428571e3 0.71428571e3 0.71428571e3 0.4 0.2 0.004 0.004 0.2 0.4'
    base_name = trial_creep
  []
  [elasticity_tensor_two]
    type = ADComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '2.0e3 2.0e5 2.0e3 0.71428571e3 0.71428571e3 0.71428571e3 0.4 0.2 0.004 0.004 0.2 0.4'
    base_name = trial_creep_two
  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_creep trial_creep_two"
    max_iterations = 5
    absolute_tolerance = 1e-05
  []

  [hill_tensor]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.5 0.5 0.3866 1.6413 1.6413 1.2731"
    base_name = trial_creep
  []

  [trial_creep]
    type = ADHillCreepStressUpdate
    coefficient = 3e-18
    n_exponent = 5
    m_exponent = 0
    activation_energy = 0
    max_inelastic_increment = 1.0e-5
    base_name = trial_creep
    # Force it to not use integration error
    max_integration_error = 1.0
  []

  [hill_tensor_two]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.5 0.5 0.3866 1.6413 1.6413 1.2731"
    base_name = trial_creep_two
  []

  [trial_creep_two]
    type = ADHillCreepStressUpdate
    coefficient = 3e-18
    n_exponent = 5
    m_exponent = 0
    activation_energy = 0
    max_inelastic_increment = 1.0e-5
    base_name = trial_creep_two
    # Force it to not use integration error
    max_integration_error = 1.0
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []

  [no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  [Pressure]
    [Side1]
      boundary = top
      function = pull
    []
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

  solve_type = NEWTON
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101                asm      lu'

  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1.0e-14
  l_max_its = 90
  num_steps = 7
  start_time = 0
  automatic_scaling = true

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 30
    iteration_window = 9
    growth_factor = 2.0
    cutback_factor = 0.5
    timestep_limiting_postprocessor = matl_ts_min
    dt = 5.0e1
  []
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
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]

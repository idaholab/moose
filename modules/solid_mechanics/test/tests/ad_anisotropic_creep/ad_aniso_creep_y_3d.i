[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 10
    nz = 2

    xmin = 0.0
    ymin = 0.0
    zmin = 0.0

    xmax = 1.0
    ymax = 10.0
    zmax = 1.0
  []
  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = '100'
    nodes = '4 10'
    input = gen
  []
  [corner_node_2]
    type = ExtraNodesetGenerator
    new_boundary = '101'
    nodes = '1 67'
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
  [creep_strain_zz]
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
  [creep_strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xx
    index_i = 0
    index_j = 0
  []
  [creep_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xy
    index_i = 0
    index_j = 1
  []
  [creep_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_yy
    index_i = 1
    index_j = 1
  []
  [creep_strain_zz]
    type = ADRankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_yy
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
    x = '0 1.0e-11 1.0'
    y = '0 -4e1 -4e1'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    generate_output = 'elastic_strain_yy stress_yy'
    use_automatic_differentiation = true
    add_variables = true
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 700
    poissons_ratio = 0.0
  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_creep_two"
    max_iterations = 50
    absolute_tolerance = 1e-16
  []

  [hill_tensor]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.5 0.25 0.3866 1.6413 1.6413 1.2731"
  []

  [trial_creep_two]
    type = ADHillCreepStressUpdate
    coefficient = 1e-16
    n_exponent = 9
    m_exponent = 0
    activation_energy = 0
    max_inelastic_increment = 0.00003
    absolute_tolerance = 1e-20
    relative_tolerance = 1e-20
    # Force it to not use integration error
    max_integration_error = 0.000001
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 101
    value = 0.0
  []

  [no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  [no_disp_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = 100
    value = 0.0
  []

  [pressure]
    type = ADPressure
    boundary = top
    function = pull
    variable = disp_y
    component = 1
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  nl_rel_tol = 1.0e-13
  nl_abs_tol = 1.0e-13
  l_max_its = 90
  num_steps = 10
  dt = 1.0e-4
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
  [creep_strain_yy]
    type = ElementalVariableValue
    variable = creep_strain_yy
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
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]

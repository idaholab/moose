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
  [temperature]
    order = CONSTANT
    family = MONOMIAL
  []
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
  [creep_strain_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [creep_strain_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [hill_constants_f]
    order = CONSTANT
    family = MONOMIAL
  []
  [hill_constants_g]
    order = CONSTANT
    family = MONOMIAL
  []
  [hill_constants_h]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [temperature]
    type = FunctionAux
    variable = temperature
    function = time_temperature
  []
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
    variable = creep_strain_zz
    index_i = 2
    index_j = 2
  []
  [creep_strain_xz]
    type = ADRankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xz
    index_i = 0
    index_j = 2
  []
  [creep_strain_yz]
    type = ADRankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_yz
    index_i = 1
    index_j = 2
  []
  [sigma_xx]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 1
    index_j = 1
  []
  [hill_constant_f]
    type = MaterialStdVectorAux
    property = hill_constants
    variable = hill_constants_f
    index = 0
  []
  [hill_constant_g]
    type = MaterialStdVectorAux
    property = hill_constants
    variable = hill_constants_g
    index = 1
  []
  [hill_constant_h]
    type = MaterialStdVectorAux
    property = hill_constants
    variable = hill_constants_h
    index = 2
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temperature
    value = 50.0
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 1.0e-9 1.0'
    y = '0 -4e1 -4e1'
  []
  [F]
    type = PiecewiseLinear
    x = '50 200'
    y = '0.2 0.5'
  []
  [G]
    type = PiecewiseLinear
    x = '50 200'
    y = '0.9 0.6'
  []
  [H]
    type = PiecewiseLinear
    x = '50 200'
    y = '0.5 0.3'
  []
  [L]
    type = PiecewiseLinear
    x = '50 200'
    y = '1.5 1.5'
  []
  [M]
    type = PiecewiseLinear
    x = '50 200'
    y = '1.5 1.5'
  []
  [N]
    type = PiecewiseLinear
    x = '50 200'
    y = '1.5 1.5'
  []
  [time_temperature]
    type = PiecewiseLinear
    x = '0 1.0e-2'
    y = '50 200'
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
    youngs_modulus = 700
    poissons_ratio = 0.0
  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'trial_creep_aniso_iso'
    max_iterations = 50
  []
  [hill_constants]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.5 0.5 0.5 1.5 1.5 1.5"
    function_names = 'F G H L M N'
    temperature = temperature
  []
  [trial_creep_aniso_iso]
    type = ADHillCreepStressUpdate
    coefficient = 1e-16
    n_exponent = 9
    m_exponent = 0
    activation_energy = 0
    max_inelastic_increment = 0.00003
    relative_tolerance = 1e-20
    absolute_tolerance = 1e-20
    internal_solve_output_on = never
    # Force it to not use integration error
    max_integration_error = 1.0
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

  nl_rel_tol = 1e-13
  nl_abs_tol = 1.0e-14
  l_max_its = 90
  num_steps = 20
  dt = 5.0e-4
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
  [creep_strain_xx]
    type = ElementalVariableValue
    variable = creep_strain_xx
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [creep_strain_yy]
    type = ElementalVariableValue
    variable = creep_strain_yy
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [creep_strain_zz]
    type = ElementalVariableValue
    variable = creep_strain_zz
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [creep_strain_xy]
    type = ElementalVariableValue
    variable = creep_strain_xy
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [creep_strain_yz]
    type = ElementalVariableValue
    variable = creep_strain_yz
    execute_on = 'TIMESTEP_END'
    elementid = 39
  []
  [creep_strain_xz]
    type = ElementalVariableValue
    variable = creep_strain_xz
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

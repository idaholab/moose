[GlobalParams]
  temperature = temp
  order = FIRST
  family = LAGRANGE
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = plane1_mesh.e
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
  group_variables = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./temp]
    initial_condition = 1500.0
  [../]
  [./creep]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./pressure]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./invariant3]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./creep_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./elastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
    extra_vector_tags = 'ref'
  [../]
[]

[AuxKernels]
  [./creep_aux]
    type = MaterialRealAux
    property = effective_creep_strain
    variable = creep
  [../]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vonmises
    scalar_type = VonMisesStress
  [../]
  [./pressure]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = pressure
    scalar_type = Hydrostatic
  [../]
  [./invariant3]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = invariant3
    scalar_type = ThirdInvariant
  [../]
  [./creep_strain_xx]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./creep_strain_yy]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_yy
    index_i = 1
    index_j = 1
  [../]
  [./creep_strain_zz]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_zz
    index_i = 2
    index_j = 2
  [../]
  [./creep_strain_xy]
    type = RankTwoAux
    rank_two_tensor = creep_strain
    variable = creep_strain_xy
    index_i = 0
    index_j = 1
  [../]
  [./elastic_str_xx_aux]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./elastic_str_yy_aux]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_yy
    index_i = 1
    index_j = 1
  [../]
  [./elastic_str_zz_aux]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_zz
    index_i = 2
    index_j = 2
  [../]
[]

[BCs]
  [./bot_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./side_x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0.0
  [../]
  [./top_press]
    type = Pressure
    variable = disp_y
    boundary = 3
    factor = -100.0
  [../]
  [./side_press]
    type = Pressure
    variable = disp_x
    boundary = 4
    factor = -200.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 200e3
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputePlaneFiniteStrain
    block = 1
  [../]
  [./radial_return_stress]
    type = ComputeMultipleInelasticStress
    block = 1
    inelastic_models = 'powerlawcrp'
  [../]
  [./powerlawcrp]
    type = PowerLawCreepStressUpdate
    block = 1
    coefficient = 3.125e-14
    n_exponent = 5.0
    m_exponent = 0.0
    activation_energy = 0.0
    max_inelastic_increment = 0.01
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       superlu_dist'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 100
  end_time = 1000.0
  num_steps = 10000
  l_tol = 1e-3

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-6
    time_t = '1e-6  2e-6 3e-6 5e-6 9e-6 1.7e-5 3.3e-5 6.5e-5 1.29e-4 2.57e-4 5.13e-4 1.025e-3 2.049e-3 4.097e-3 8.193e-3 1.638e-2 3.276e-2 5.734e-2 0.106 0.180 0.291 0.457 0.706 1.08 1.64 2.48 3.74 5.63 8.46 12.7 19.1 28.7 43.0 64.5 108.0 194.0 366.0 710.0 1000.0'
    time_dt = '1e-6 1e-6 2e-6 4e-6 8e-6 1.6e-5 3.2e-5 6.4e-5 1.28e-4 2.56e-4 5.12e-4 1.024e-3 2.048e-3 4.096e-3 8.192e-3 1.6384e-2 2.458e-2 4.915e-2 7.40e-2 0.111 0.166 0.249 0.374 0.560 0.840 1.26 1.89 2.83 4.25 6.40 9.6 14.3 21.5 43.0 86.1 172.0 344.0 290.0 290.0'
    optimal_iterations = 30
    iteration_window = 9
    growth_factor = 2.0
    cutback_factor = 0.5
    timestep_limiting_postprocessor = matl_ts_min
  [../]
[]

[Postprocessors]
  [./matl_ts_min]
    type = MaterialTimeStepPostprocessor
  [../]
  [./sigma_xx]
    type = ElementAverageValue
    variable = stress_xx
  [../]
  [./sigma_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./sigma_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./vonmises]
    type = ElementAverageValue
    variable = vonmises
  [../]
  [./pressure]
    type = ElementAverageValue
    variable = pressure
  [../]
  [./invariant3]
    type = ElementAverageValue
    variable = invariant3
  [../]
  [./eps_crp_xx]
    type = ElementAverageValue
    variable = creep_strain_xx
  [../]
  [./eps_crp_yy]
    type = ElementAverageValue
    variable = creep_strain_yy
  [../]
  [./eps_crp_zz]
    type = ElementAverageValue
    variable = creep_strain_zz
  [../]
  [./eps_crp_mag]
    type = ElementAverageValue
    variable = creep
  [../]
  [./disp_x2]
    type = NodalVariableValue
    nodeid = 1
    variable = disp_x
  [../]
  [./disp_x3]
    type = NodalVariableValue
    nodeid = 2
    variable = disp_x
  [../]
  [./disp_y3]
    type = NodalVariableValue
    nodeid = 2
    variable = disp_y
  [../]
  [./disp_y4]
    type = NodalVariableValue
    nodeid = 3
    variable = disp_y
  [../]
  [./_dt]
    type = TimestepSize
  [../]
  [./elas_str_xx]
    type = ElementAverageValue
    variable = elastic_strain_xx
  [../]
  [./elas_str_yy]
    type = ElementAverageValue
    variable = elastic_strain_yy
  [../]
  [./elas_str_zz]
    type = ElementAverageValue
    variable = elastic_strain_zz
  [../]
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  csv = true
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 25
  [../]
[]

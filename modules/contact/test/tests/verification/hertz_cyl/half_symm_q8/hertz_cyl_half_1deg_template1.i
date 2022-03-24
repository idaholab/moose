[GlobalParams]
  order = SECOND
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = hertz_cyl_half_1deg.e
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./diag_saved_x]
  [../]
  [./diag_saved_y]
  [../]
  [./inc_slip_x]
  [../]
  [./inc_slip_y]
  [../]
  [./accum_slip_x]
  [../]
  [./accum_slip_y]
  [../]
  [./tang_force_x]
  [../]
  [./tang_force_y]
  [../]
[]

[Functions]
  [./disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. -0.0020 -0.0020'
  [../]
  [./disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. 0.0 0.0014'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  [../]
  [./inc_slip_x]
    type = PenetrationAux
    variable = inc_slip_x
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./inc_slip_y]
    type = PenetrationAux
    variable = inc_slip_y
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./accum_slip_x]
    type = PenetrationAux
    variable = accum_slip_x
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./accum_slip_y]
    type = PenetrationAux
    variable = accum_slip_y
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./tang_force_x]
    type = PenetrationAux
    variable = tang_force_x
    quantity = tangential_force_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./tang_force_y]
    type = PenetrationAux
    variable = tang_force_y
    quantity = tangential_force_y
    boundary = 3
    paired_boundary = 2
  [../]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
  [../]
[]

[Postprocessors]
  [./bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 1
  [../]
  [./bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  [../]
  [./top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 4
  [../]
  [./top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 4
  [../]
  [./disp_x639]
    type = NodalVariableValue
    nodeid = 638
    variable = disp_x
  [../]
  [./disp_y639]
    type = NodalVariableValue
    nodeid = 638
    variable = disp_y
  [../]
  [./_dt]
    type = TimestepSize
  [../]
  [./num_lin_it]
    type = NumLinearIterations
  [../]
  [./num_nonlin_it]
    type = NumNonlinearIterations
  [../]
[]

[BCs]
  [./side_x]
    type = DirichletBC
    variable = disp_y
    boundary = '1 2'
    value = 0.0
  [../]
  [./bot_y]
    type = DirichletBC
    variable = disp_x
    boundary = '1 2'
    value = 0.0
  [../]
  [./top_y_disp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = disp_ramp_vert
  [../]
  [./top_x_disp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = disp_ramp_horz
  [../]
[]

[Materials]
  [./stuff1_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e10
    poissons_ratio = 0.0
  [../]
  [./stuff1_strain]
    type = ComputeFiniteStrain
    block = '1'
  [../]
  [./stuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  [../]
  [./stuff2_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stuff2_strain]
    type = ComputeFiniteStrain
    block = '2'
  [../]
  [./stuff2_stress]
    type = ComputeFiniteStrainElasticStress
    block = '2'
  [../]
  [./stuff3_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '3'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stuff3_strain]
    type = ComputeFiniteStrain
    block = '3'
  [../]
  [./stuff3_stress]
    type = ComputeFiniteStrainElasticStress
    block = '3'
  [../]
  [./stuff4_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '4'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stuff4_strain]
    type = ComputeFiniteStrain
    block = '4'
  [../]
  [./stuff4_stress]
    type = ComputeFiniteStrainElasticStress
    block = '4'
  [../]
  [./stuff5_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '5'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stuff5_strain]
    type = ComputeFiniteStrain
    block = '5'
  [../]
  [./stuff5_stress]
    type = ComputeFiniteStrainElasticStress
    block = '5'
  [../]
  [./stuff6_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '6'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stuff6_strain]
    type = ComputeFiniteStrain
    block = '6'
  [../]
  [./stuff6_stress]
    type = ComputeFiniteStrainElasticStress
    block = '6'
  [../]
  [./stuff7_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '7'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stuff7_strain]
    type = ComputeFiniteStrain
    block = '7'
  [../]
  [./stuff7_stress]
    type = ComputeFiniteStrainElasticStress
    block = '7'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-5
  l_max_its = 100
  nl_max_its = 200

  start_time = 0.0
  end_time = 3.5
  l_tol = 1e-3
  dt = 0.1
  dtmin = 0.1
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[VectorPostprocessors]
  [./x_disp]
    type = NodalValueSampler
    variable = disp_x
    boundary = '3 4'
    sort_by = id
  [../]
  [./y_disp]
    type = NodalValueSampler
    variable = disp_y
    boundary = '3 4'
    sort_by = id
  [../]
  [./cont_press]
    type = NodalValueSampler
    variable = contact_pressure
    boundary = '3'
    sort_by = id
  [../]
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
  [./chkfile]
    type = CSV
    show = 'x_disp y_disp cont_press'
    start_time = 0.9
    execute_vector_postprocessors_on = timestep_end
  [../]
  [./chkfile2]
    type = CSV
    show = 'bot_react_x bot_react_y disp_x639 disp_y639 top_react_x top_react_y'
    start_time = 0.9
    execute_vector_postprocessors_on = timestep_end
  [../]
  [./outfile]
    type = CSV
    delimiter = ' '
    execute_vector_postprocessors_on = none
  [../]
[]

[Contact]
  [./interface]
    primary = 2
    secondary = 3
    normalize_penalty = true
    tangential_tolerance = 1e-3
    penalty = 1e+10
  [../]
[]

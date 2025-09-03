[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = plane2_mesh.e
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_residuals = 'ref'
  reference_residual = 'ref'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [penetration]
  []
  [saved_x]
  []
  [saved_y]
  []
  [diag_saved_x]
  []
  [diag_saved_y]
  []
  [inc_slip_x]
  []
  [inc_slip_y]
  []
  [accum_slip_x]
  []
  [accum_slip_y]
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = true
    save_in = 'saved_x saved_y'
    extra_residual_tags = 'ref'
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [zeroslip_x]
    type = ConstantAux
    variable = inc_slip_x
    boundary = 4
    execute_on = timestep_begin
    value = 0.0
  []
  [zeroslip_y]
    type = ConstantAux
    variable = inc_slip_y
    boundary = 4
    execute_on = timestep_begin
    value = 0.0
  []
  [accum_slip_x]
    type = AccumulateAux
    variable = accum_slip_x
    accumulate_from_variable = inc_slip_x
    execute_on = timestep_end
  []
  [accum_slip_y]
    type = AccumulateAux
    variable = accum_slip_y
    accumulate_from_variable = inc_slip_y
    execute_on = timestep_end
  []
  [penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 4
    paired_boundary = 3
  []
[]

[Postprocessors]
  [bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 1
  []
  [bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  []
  [top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 5
  []
  [top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 5
  []
  [ref_resid_x]
    type = NodalL2Norm
    execute_on = timestep_end
    variable = saved_x
  []
  [ref_resid_y]
    type = NodalL2Norm
    execute_on = timestep_end
    variable = saved_y
  []
  [sigma_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [sigma_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [disp_x5]
    type = NodalVariableValue
    nodeid = 4
    variable = disp_x
  []
  [disp_x9]
    type = NodalVariableValue
    nodeid = 8
    variable = disp_x
  []
  [disp_y5]
    type = NodalVariableValue
    nodeid = 4
    variable = disp_y
  []
  [disp_y9]
    type = NodalVariableValue
    nodeid = 8
    variable = disp_y
  []
  [_dt]
    type = TimestepSize
  []
  [num_lin_it]
    type = NumLinearIterations
  []
  [num_nonlin_it]
    type = NumNonlinearIterations
  []
[]

[BCs]
  [bot_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  []
  [side_x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0.0
  []
  [top_press]
    type = Pressure
    variable = disp_y
    boundary = 5
    factor = 109.89
  []
[]

[Materials]
  [bot_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [bot_strain]
    type = ComputeIncrementalStrain
    block = '1'
  []
  [bot_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
  [top_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [top_strain]
    type = ComputeIncrementalStrain
    block = '2'
  []
  [top_stress]
    type = ComputeFiniteStrainElasticStress
    block = '2'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-7
  l_max_its = 100
  nl_max_its = 200
  dt = 1.0
  end_time = 1.0
  num_steps = 10
  dtmin = 1.0
  l_tol = 1e-3
[]

[VectorPostprocessors]
  [x_disp]
    type = NodalValueSampler
    variable = disp_x
    boundary = '1 3 4 5'
    sort_by = x
  []
  [cont_press]
    type = NodalValueSampler
    variable = contact_pressure
    boundary = '3'
    sort_by = x
  []
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  [exodus]
    type = Exodus
    elemental_as_nodal = true
  []
  [console]
    type = Console
    max_rows = 5
  []
  [chkfile]
    type = CSV
    show = 'bot_react_x bot_react_y disp_x5 disp_y5 disp_x9 disp_y9 sigma_yy sigma_zz top_react_x top_react_y x_disp cont_press'
    execute_vector_postprocessors_on = timestep_end
  []
  [outfile]
    type = CSV
    delimiter = ' '
    execute_vector_postprocessors_on = none
  []
[]

[Contact]
  [leftright]
    secondary = 3
    primary = 4
    normalize_penalty = true
    tangential_tolerance = 1e-3
    penalty = 1e+9
  []
[]

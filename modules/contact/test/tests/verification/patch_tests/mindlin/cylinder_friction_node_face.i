[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [input_file]
    type = FileMeshGenerator
    file = hertz_cyl_coarser.e
  []
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
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
  [react_x]
  []
  [react_y]
  []
  [penetration]
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

[Functions]
  [disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. -0.020 -0.020'
  []
  [disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. 0.0 0.015'
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = true
    extra_vector_tags = 'ref'
    block = '1 2 3 4 5 6 7'
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
    block = '1 2 3 4 5 6 7'
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
    block = '1 2 3 4 5 6 7'
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
    block = '1 2 3 4 5 6 7'
  []
  [incslip_x]
    type = PenetrationAux
    variable = inc_slip_x
    quantity = incremental_slip_x
    boundary = 3
    paired_boundary = 2
  []
  [incslip_y]
    type = PenetrationAux
    variable = inc_slip_y
    quantity = incremental_slip_y
    boundary = 3
    paired_boundary = 2
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
    boundary = 3
    paired_boundary = 2
  []
  [react_x]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_x'
    variable = 'react_x'
  []
  [react_y]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_y'
    variable = 'react_y'
  []
[]

[Postprocessors]
  [bot_react_x]
    type = NodalSum
    variable = react_x
    boundary = 1
  []
  [bot_react_y]
    type = NodalSum
    variable = react_y
    boundary = 1
  []
  [top_react_x]
    type = NodalSum
    variable = react_x
    boundary = 4
  []
  [top_react_y]
    type = NodalSum
    variable = react_y
    boundary = 4
  []
  [penetration]
    type = NodalExtremeValue
    variable = penetration
    value_type = max
    boundary = 3
  []
  [inc_slip_x_max]
    type = NodalExtremeValue
    variable = inc_slip_x
    value_type = max
    boundary = 3
  []
  [inc_slip_x_min]
    type = NodalExtremeValue
    variable = inc_slip_x
    value_type = min
    boundary = 3
  []
  [inc_slip_y_max]
    type = NodalExtremeValue
    variable = inc_slip_y
    value_type = max
    boundary = 3
  []
  [inc_slip_y_min]
    type = NodalExtremeValue
    variable = inc_slip_y
    value_type = min
    boundary = 3
  []
  [accum_slip_x]
    type = NodalExtremeValue
    variable = accum_slip_x
    value_type = max
    boundary = 3
  []
  [accum_slip_y]
    type = NodalExtremeValue
    variable = accum_slip_y
    value_type = max
    boundary = 3
  []
  [_dt]
    type = TimestepSize
  []
[]

[BCs]
  [side_x]
    type = DirichletBC
    variable = disp_y
    boundary = '1 2'
    value = 0.0
  []
  [bot_y]
    type = DirichletBC
    variable = disp_x
    boundary = '1 2'
    value = 0.0
  []
  [top_y_disp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = disp_ramp_vert
  []
  [top_x_disp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = disp_ramp_horz
  []
[]

[Materials]
  [stuff1_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e10
    poissons_ratio = 0.0
  []
  [stuff1_strain]
    type = ComputeFiniteStrain
    block = '1'
  []
  [stuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
  [stuff2_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2 3 4 5 6 7'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stuff2_strain]
    type = ComputeFiniteStrain
    block = '2 3 4 5 6 7'
  []
  [stuff2_stress]
    type = ComputeFiniteStrainElasticStress
    block = '2 3 4 5 6 7'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       superlu_dist                  NONZERO               1e-15          '
                        '         1e-5'

  line_search = 'none'

  nl_abs_tol = 1e-8
  start_time = 0.0
  end_time = 0.3
  l_tol = 1e-4
  dt = 0.1
  dtmin = 0.1
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[VectorPostprocessors]
  [x_disp]
    type = NodalValueSampler
    variable = disp_x
    boundary = '3 4'
    sort_by = id
  []
  [y_disp]
    type = NodalValueSampler
    variable = disp_y
    boundary = '3 4'
    sort_by = id
  []
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  exodus = true
  csv = true
  [console]
    type = Console
    max_rows = 5
  []
  [chkfile]
    type = CSV
    show = 'x_disp y_disp'
    file_base = cylinder_friction_check
    create_final_symlink = true
    execute_on = 'FINAL'
  []
[]

[Contact]
  [leftright]
    primary = 2
    secondary = 3
    model = coulomb
    formulation = penalty
    penalty = 5e9
    normalize_penalty = true
    friction_coefficient = '0.2'
  []
[]

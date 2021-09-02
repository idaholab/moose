[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [input_file]
    type = FileMeshGenerator
    file = cylinder.e
  []
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
[]

[AuxVariables]
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
  [tang_force_x]
  []
  [tang_force_y]
  []
  [nodal_area]
  []
[]

[Functions]
  [disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. -0.025 -0.025'
  []
  [disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 1. 3.5'
    y = '0. 0.0 0.01'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = SMALL
    generate_output = 'stress_xx stress_yy stress_xy vonmises_stress hydrostatic_stress'
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
    block = '1 2 3 4 5 6 7'
  []
[]

[UserObjects]
  [nodal_area]
    type = NodalArea
    variable = nodal_area
    boundary = 3
    execute_on = 'initial timestep_end'
  []
[]

[Contact]
  [friction]
    primary = 2
    secondary = 3
    model = coulomb
    formulation = tangential_penalty
    friction_coefficient = 0.7
    penalty = 4e+02
  []
[]

[AuxKernels]
  [inc_slip_x]
    type = PenetrationAux
    variable = inc_slip_x
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  []
  [inc_slip_y]
    type = PenetrationAux
    variable = inc_slip_y
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  []
  [accum_slip_x]
    type = PenetrationAux
    variable = accum_slip_x
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  []
  [accum_slip_y]
    type = PenetrationAux
    variable = accum_slip_y
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  []
  [penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
  []
  [tang_force_x]
    type = PenetrationAux
    variable = tang_force_x
    quantity = tangential_force_x
    boundary = 3
    paired_boundary = 2
  []
  [tang_force_y]
    type = PenetrationAux
    variable = tang_force_y
    quantity = tangential_force_y
    boundary = 3
    paired_boundary = 2
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
    boundary = 4
  []
  [top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 4
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
  [tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.0e10
    poissons_ratio = 0.0
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1'
  []
  [tensor_cylinder]
    type = ComputeIsotropicElasticityTensor
    block = '2 3 4 5 6 7'
    youngs_modulus = 7000
    poissons_ratio = 0.3
  []
  [stress_cylinder]
    type = ComputeLinearElasticStress
    block = '2 3 4 5 6 7'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       NONZERO               1e-15                   1e-5'

  line_search = 'none'

  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-5
  l_max_its = 100
  nl_max_its = 150

  start_time = 0.0
  end_time = 0.25 # 3.5
  l_tol = 1e-4
  dt = 0.25
  dtmin = 0.001
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
    sort_by = x
  []
  [y_disp]
    type = NodalValueSampler
    variable = disp_y
    boundary = '3 4'
    sort_by = x
  []
  [cont_press]
    type = NodalValueSampler
    variable = contact_pressure
    boundary = '3'
    sort_by = x
  []
  [friction_force]
    type = NodalValueSampler
    variable = tang_force_x
    boundary = '3'
    sort_by = x
  []
  [nodal_area_post]
    type = NodalValueSampler
    variable = nodal_area
    boundary = '3'
    sort_by = x
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
    show = 'x_disp y_disp cont_press friction_force'
    start_time = 0.0
    execute_vector_postprocessors_on = FINAL
  []
[]

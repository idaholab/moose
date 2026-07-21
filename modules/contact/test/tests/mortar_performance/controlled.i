cycles = 800
period = 2
dt = 1
name = controlled
normal_lm_guess = 0
tangential_lm_guess = 0

[GlobalParams]
  displacements = 'disp_x disp_y'
  scaling = 1
[]

[Mesh]
  [top]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 1
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 2
  []
  [top_boundaries]
    type = RenameBoundaryGenerator
    input = top
    old_boundary = '0 1 2 3'
    new_boundary = 'top_bottom top_right top_top top_left'
  []
  [top_id]
    type = SubdomainIDGenerator
    input = top_boundaries
    subdomain_id = 1
  []
  [bottom]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 1
    xmin = 0
    xmax = 4
    ymin = -2
    ymax = 0
  []
  [bottom_boundaries]
    type = RenameBoundaryGenerator
    input = bottom
    old_boundary = '0 1 2 3'
    new_boundary = '100 101 102 103'
  []
  [bottom_id]
    type = SubdomainIDGenerator
    input = bottom_boundaries
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'top_id bottom_id'
  []
  [bottom_boundary_names]
    type = RenameBoundaryGenerator
    input = combined
    old_boundary = '100 101 102 103'
    new_boundary = 'bottom_bottom bottom_right bottom_top bottom_left'
  []
  [blocks]
    type = RenameBlockGenerator
    input = bottom_boundary_names
    old_block = '1 2'
    new_block = 'top bottom'
  []
  allow_renumbering = false
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    block = 'top bottom'
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = 'top bottom'
    youngs_modulus = 4
    poissons_ratio = 0
  []
  [stress]
    type = ComputeLinearElasticStress
    block = 'top bottom'
  []
[]

[Functions]
  [normal_history]
    type = ParsedFunction
    expression = '-0.1 * min(t, 1)'
  []
  [tangential_history]
    type = ParsedFunction
    expression = 'if(t < 2, 0, 0.1 * cos(pi * (t - 2)))'
  []
[]

[Contact]
  [mortar]
    primary = bottom_top
    secondary = top_bottom
    formulation = mortar
    model = coulomb
    friction_coefficient = 0.5
    c_normal_strategy = physical
    c_tangential_strategy = physical
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom_bottom
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom_bottom
    value = 0
  []
  [top_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top_top
    function = tangential_history
  []
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top_top
    function = normal_history
  []
[]

[ICs]
  [normal_lm]
    type = FunctionIC
    variable = mortar_normal_lm
    function = 'if(x<0.5,0,if(x>3.5,0,${normal_lm_guess}))'
  []
  [tangential_lm]
    type = FunctionIC
    variable = mortar_tangential_lm
    function = 'if(x<0.5,0,if(x>3.5,0,${tangential_lm_guess}))'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = ${dt}
  dtmin = ${dt}
  end_time = '${fparse cycles * period}'
  abort_on_solve_fail = true
  nl_max_its = 50
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  line_search = bt
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [kkt_error]
    type = FrictionKKTErrorPostprocessor
    weighted_velocities_uo = lm_weightedvelocities_object_mortar
    normal_lm = mortar_normal_lm
    tangential_lm = mortar_tangential_lm
    friction_coefficient = 0.5
    normal_scale = 1
    tangential_scale = 1
    execute_on = timestep_end
  []
  [contact]
    type = ContactDOFSetSize
    variable = mortar_normal_lm
    subdomain = mortar_secondary_subdomain
    execute_on = 'nonlinear timestep_end'
  []
  [normal_pressure]
    type = ElementAverageValue
    variable = mortar_normal_lm
    block = mortar_secondary_subdomain
  []
  [tangential_pressure]
    type = ElementAverageValue
    variable = mortar_tangential_lm
    block = mortar_secondary_subdomain
  []
  [num_nonlinear_iterations]
    type = NumNonlinearIterations
  []
  [total_nonlinear_iterations]
    type = CumulativeValuePostprocessor
    postprocessor = num_nonlinear_iterations
  []
  [num_residual_evaluations]
    type = NumResidualEvaluations
  []
  [num_linear_iterations]
    type = NumLinearIterations
  []
  [total_linear_iterations]
    type = CumulativeValuePostprocessor
    postprocessor = num_linear_iterations
  []
  [disp_x_residual]
    type = DiscreteVariableResidualNorm
    variable = disp_x
    norm_type = l_inf
    execute_on = timestep_end
  []
  [disp_y_residual]
    type = DiscreteVariableResidualNorm
    variable = disp_y
    norm_type = l_inf
    execute_on = timestep_end
  []
  [timestep_size]
    type = TimestepSize
  []
[]

[Outputs]
  file_base = ${name}
  [csv]
    type = CSV
  []
[]

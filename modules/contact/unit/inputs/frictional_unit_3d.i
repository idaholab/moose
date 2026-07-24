normal_load = '-0.1*t'
tangential_x_load = '0.2*t'
tangential_y_load = '0.2*t'

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  scaling = 1
[]

[Mesh]
  [top]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 1
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 2
    zmin = 0
    zmax = 2
  []
  [top_boundaries]
    type = RenameBoundaryGenerator
    input = top
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'top_bottom top_back top_right top_front top_left top_top'
  []
  [top_id]
    type = SubdomainIDGenerator
    input = top_boundaries
    subdomain_id = 1
  []
  [bottom]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 1
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 2
    zmin = -2
    zmax = 0
  []
  [bottom_boundaries]
    type = RenameBoundaryGenerator
    input = bottom
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '100 101 102 103 104 105'
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
    old_boundary = '100 101 102 103 104 105'
    new_boundary = 'bottom_bottom bottom_back bottom_right bottom_front bottom_left bottom_top'
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
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 -0.1 -0.1 -0.1'
  []
  [tangential_x_history]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0.2 0 -0.2'
  []
  [tangential_y_history]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 0.2 0'
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
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom_bottom
    value = 0
  []
  [top_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top_top
    function = '${tangential_x_load}'
  []
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top_top
    function = '${tangential_y_load}'
  []
  [top_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top_top
    function = '${normal_load}'
  []
[]

[ICs]
  [normal_lm]
    type = ConstantIC
    variable = mortar_normal_lm
    value = 0
  []
  [tangential_lm]
    type = ConstantIC
    variable = mortar_tangential_lm
    value = 0
  []
  [tangential_3d_lm]
    type = ConstantIC
    variable = mortar_tangential_3d_lm
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 1
  dtmin = 1
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
    type = FrictionNaturalMapPostprocessor
    weighted_velocities_uo = lm_weightedvelocities_object_mortar
    normal_lm = mortar_normal_lm
    tangential_lm = mortar_tangential_lm
    tangential_lm_two = mortar_tangential_3d_lm
    friction_coefficient = 0.5
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
  [tangential_pressure_two]
    type = ElementAverageValue
    variable = mortar_tangential_3d_lm
    block = mortar_secondary_subdomain
  []
  [top_displacement]
    type = ElementAverageValue
    variable = disp_z
    block = top
  []
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [residual_evaluations]
    type = NumResidualEvaluations
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
  [disp_z_residual]
    type = DiscreteVariableResidualNorm
    variable = disp_z
    norm_type = l_inf
    execute_on = timestep_end
  []
[]

[Outputs]
  [console]
    type = Console
    max_rows = 2
  []
[]

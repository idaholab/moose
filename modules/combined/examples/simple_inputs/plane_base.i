[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [bottom]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin=0
    xmax=1
    ymin=0
    ymax=0.5
  []
  [top]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin=0.4
    xmax=0.6
    ymin=0.5
    ymax=0.6
  []
  [rename_top]
    type = RenameBlockGenerator
    input = top
    old_block_id = 0
    new_block_id = 1
  []
  [collect_mesh]
    type = MeshCollectionGenerator
    inputs = 'bottom rename_top'
  []
  [side_bot2]
    type = SideSetsAroundSubdomainGenerator
    input = collect_mesh
    block = 1
    new_boundary = '4'
    normal = '0 -1 0'
    replace = true
  []
  [side_right2]
    type = SideSetsAroundSubdomainGenerator
    input = side_bot2
    block = 1
    new_boundary = '5'
    normal = '1 0 0'
    replace = true
  []
  [side_top2]
    type = SideSetsAroundSubdomainGenerator
    input = side_right2
    block = 1
    new_boundary = '6'
    normal = '0 1 0'
    replace = true
  []
  [side_left2]
    type = SideSetsAroundSubdomainGenerator
    input = side_top2
    block = 1
    new_boundary = '7'
    normal = '-1 0 0'
    replace = true
  []
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
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
    order = FIRST
    family = LAGRANGE
  []
  [saved_x]
  []
  [saved_y]
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = true
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
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
  [penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 4
    paired_boundary = 2
  []
[]

[Postprocessors]
  [penetration]
    type = ElementAverageValue
    variable = penetration
  []
  [sigma_xx]
    type = ElementAverageValue
    variable = stress_xx
  []
  [sigma_xy]
    type = ElementAverageValue
    variable = stress_xy
  []
  [sigma_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [sigma_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 0
  []
  [bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 0
  []
  [top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 6
  []
  [top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 6
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

[Functions]
  [disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. -0.002'
  []
  [disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 0.10'
  []
[]

[BCs]
  [bot_y]
    type = DirichletBC
    variable = disp_y
    boundary = 0
    value = 0.0
  []
  [side_x]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0.0
  []
  [top_disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 6
    function = disp_ramp_vert
  []
[]

[Materials]
  [bot_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '0'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [bot_strain]
    type = ComputeIncrementalSmallStrain
    block = '0'
  []
  [bot_stress]
    type = ComputeFiniteStrainElasticStress
    block = '0'
  []
  [top_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [top_strain]
    type = ComputeIncrementalSmallStrain
    block = '1'
  []
  [top_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
[]

[Contact]
  [interface]
    secondary = 4
    primary = 2
    model = frictionless
    formulation = penalty
    penalty = 1e+3
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-7
  l_max_its = 20
  nl_max_its = 40
  dt = 0.1
  end_time = 1.0
  num_steps = 1000
  dtmin = 0.01
  l_tol = 1e-3
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  csv = true
  [exodus]
    type = Exodus
    elemental_as_nodal = true
  []
  [console]
    type = Console
    max_rows = 5
  []
[]

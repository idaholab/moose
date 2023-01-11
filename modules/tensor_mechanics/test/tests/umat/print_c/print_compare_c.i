[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
[]

[Functions]
  [top_pull]
    type = ParsedFunction
    expression = -t/1000
  []
[]

[AuxVariables]
  [strain_xy]
    family = MONOMIAL
    order = FIRST
  []
  [strain_yy]
    family = MONOMIAL
    order = FIRST
  []
[]

[AuxKernels]
  [strain_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xy
    index_i = 1
    index_j = 0
  []
  [strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
  []
[]

[BCs]
  [x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
[]

[NodalKernels]
  [force_x]
    type = ConstantRate
    variable = disp_x
    boundary = top
    rate = 1.0e0
  []
[]

[Materials]
  # 1. Active for UMAT verification
  [umat_c]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    plugin = '../../../plugins/elastic_print_c'
    num_state_vars = 0
    use_one_based_indexing = true
  []
  [umat_f]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    plugin = '../../../plugins/elastic'
    num_state_vars = 0
    use_one_based_indexing = true
  []
  [umat_eigen]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    plugin = '../../../plugins/elastic_print_eigen'
    num_state_vars = 0
    use_one_based_indexing = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
  start_time = 0.0
  end_time = 10

  dt = 10.0
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
[]

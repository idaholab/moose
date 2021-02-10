[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[XFEM]
  geometric_cut_userobjects = 'line_seg_cut_uo'
  qrule = moment_fitting
  output_cut_plane = true
[]

[UserObjects]
  [line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '0.0 0.5 1.0 0.5'
    time_start_cut = 0.0
    time_end_cut = 5.0
  []
  [pair_qps]
    type = XFEMElementPairQPProvider
  []
  [manager]
    type = XFEMElementPairMaterialManager
    material_names = 'material1'
    element_pair_qps = pair_qps
  []
[]

[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    elem_type = QUAD
  []
  [right_bottom]
    type = ExtraNodesetGenerator
    new_boundary = 'right_bottom'
    coord = '1 0'
    input = generate
  []
  [right_top]
    type = ExtraNodesetGenerator
    new_boundary = 'right_top'
    coord = '1 1'
    input = right_bottom
  []
[]

[AuxVariables]
  [resid_y]
  []
  [resid_x]
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    generate_output = 'stress_xx strain_yy vonmises_stress elastic_strain_yy'
    save_in = 'resid_x resid_y'
  []
[]

[Constraints]
  [dispx_constraint]
    type = XFEMCohesiveConstraint
    use_displaced_mesh = false
    component = 0
    variable = disp_x
    stiffness = 10
    max_traction = 0.1
    Gc = 0.0005
    manager = manager
  []
  [dispy_constraint]
    type = XFEMCohesiveConstraint
    use_displaced_mesh = false
    component = 1
    variable = disp_y
    stiffness = 10
    max_traction = 0.1
    Gc = 0.0005
    manager = manager
  []
[]

[Functions]
  [pull_up_and_down]
    type = ParsedFunction
    value = 'if(t < 5, 0.0001 * t, -0.0001 * (t-5) + 0.0005)'
  []
[]

[BCs]
  [bottomx]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [topx]
    type = DirichletBC
    boundary = top
    variable = disp_x
    value = 0.0
  []
  [topy]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = pull_up_and_down
  []
[]

[Materials]
  [linelast]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.31
    youngs_modulus = 5000.0
  []
  [material1]
    type = MaximumNormalSeparation
    compute = false
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Postprocessors]
  [react_y_top]
    type = NodalSum
    variable = resid_y
    boundary = top
  []
  [react_x_top]
    type = NodalSum
    variable = resid_x
    boundary = top
  []
  [react_y_bottom]
    type = NodalSum
    variable = resid_y
    boundary = bottom
  []
  [react_x_bottom]
    type = NodalSum
    variable = resid_x
    boundary = bottom
  []
  [react_y_left]
    type = NodalSum
    variable = resid_y
    boundary = left
  []
  [react_x_left]
    type = NodalSum
    variable = resid_x
    boundary = left
  []
  [disp_y]
    type = NodalMaxValue
    variable = disp_y
    boundary = top
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'
  line_search = 'bt'

  l_max_its = 20
  l_tol = 1e-3

  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5

  start_time = 0.0
  dt = 1
  end_time = 5
  num_steps = 5000

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  csv = true
  [console]
    type = Console
    perf_log = true
    output_linear = true
  []
[]

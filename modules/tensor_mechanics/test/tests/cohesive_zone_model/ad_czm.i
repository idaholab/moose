[Mesh]
  [msh]
    type = FileMeshGenerator
    file = patch_mesh.e
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = msh
  []
  [add_surfaces]
    type = SideSetsFromNormalsGenerator
    input = split
    normals = '0  0  1
               0  1  0
               1  0  0
               0  0 -1
               0 -1  0
              -1  0  0'
    fixed_normal = true
    new_boundary = 'z1 y1 x1 z0 y0 x0'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Modules]
  [TensorMechanics]
    [Master]
      [all]
        strain = FINITE
        add_variables = true
        new_system = true
      []
    []
    [CohesiveZoneMaster]
      [interface]
        boundary = 'interface'
        strain = SMALL
        use_automatic_differentiation = true
      []
    []
  []
[]

[Functions]
  [stretch]
    type = PiecewiseLinear
    x = '0 0.05'
    y = '0 0.1'
  []
[]

[Constraints]
  [x1]
    type = EqualValueBoundaryConstraint
    variable = disp_x
    secondary = 'x1' # boundary
    penalty = 1e6
  []
  [y1]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    secondary = 'y1' # boundary
    penalty = 1e6
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    value = 0.0
    boundary = 'x0'
    variable = disp_x
  []
  [fix_y]
    type = DirichletBC
    value = 0.0
    boundary = 'y0'
    variable = disp_y
  []
  [fix_z]
    type = DirichletBC
    value = 0.0
    boundary = 'z0'
    variable = disp_z
  []
  [back_z]
    type = FunctionDirichletBC
    boundary = 'z1'
    variable = disp_z
    use_displaced_mesh = true
    function = stretch
    preset = false
  []
  [rotate_x]
    type = DisplacementAboutAxis
    boundary = 'x0 y0 z0 x1 y1 z1'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 0
    variable = disp_x
    angular_velocity = true
    preset = false
  []
  [rotate_y]
    type = DisplacementAboutAxis
    boundary = 'x0 y0 z0 x1 y1 z1'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 1
    variable = disp_y
    angular_velocity = true
    preset = false
  []
  [rotate_z]
    type = DisplacementAboutAxis
    boundary = 'x0 y0 z0 x1 y1 z1'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 2
    variable = disp_z
    angular_velocity = true
    preset = false
  []
[]

[Controls]
  [c1]
    type = TimePeriod
    enable_objects = 'BCs::fix_x BCs::fix_y BCs::fix_z BCs::back_z Constraints::x1 Constraints::y1'
    disable_objects = 'BCs::rotate_x BCs::rotate_y BCs::rotate_z'
    start_time = '0'
    end_time = '0.05'
  []
[]

[Materials]
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
  []
  [czm_mat]
    type = ADPureElasticTractionSeparation
    normal_stiffness = 1e4
    tangent_stiffness = 7e3
    boundary = 'interface'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none
  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-15
  start_time = 0.0
  dt = 0.025
  end_time = 0.075
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

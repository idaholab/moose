#
# Stretch + rotation test
#
# This test is designed to compute a uniaxial stress and then follow it as the mesh is rotated .
#
# The mesh is composed of two, single-elemnt blocks

[Mesh]
  [./msh]
  type = GeneratedMeshGenerator
  dim = 3
  nx = 1
  ny = 1
  nz = 2
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -1
  zmax = 1
  []
  [./new_block]
    type = SubdomainBoundingBoxGenerator
    input = msh
    block_id = 1
    bottom_left = '-0.5 -0.5 0'
    top_right = '0.5 0.5 0.5'
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = new_block
  []
  [add_side_sets]
    input = split
    type = SideSetsFromNormalsGenerator
    normals = '0 -1  0
               0  1  0
               -1 0  0
               1  0  0
               0  0 -1
               0  0  1'
    fixed_normal = true
    new_boundary = 'y0 y1 x0 x1 z0 z1'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./stretch]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 300'
  [../]
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = x0
    variable = disp_x
  [../]
  [./fix_y]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = y0
    variable = disp_y
  [../]
  [./fix_z]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = z0
    variable = disp_z
  [../]
  [./back_z]
    type = FunctionNeumannBC
    boundary = z1
    variable = disp_z
    use_displaced_mesh = false
    function = stretch
  [../]

  [./rotate_x]
    type = DisplacementAboutAxis
    boundary = 'x0 y0 z0 z1'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 0
    variable = disp_x
    angular_velocity = true
  [../]
  [./rotate_y]
    type = DisplacementAboutAxis
    boundary = 'x0 y0 z0 z1'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 1
    variable = disp_y
    angular_velocity = true
  [../]
  [./rotate_z]
    type = DisplacementAboutAxis
    boundary = 'x0 y0 z0 z1'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 2
    variable = disp_z
    angular_velocity = true
  [../]
[]



[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm_ik]
    boundary = 'interface'
    strain = FINITE
    generate_output='traction_x traction_y traction_z jump_x jump_y jump_z normal_traction tangent_traction normal_jump tangent_jump pk1_traction_x pk1_traction_y pk1_traction_z'
  [../]
[]

[Controls]
  [./c1]
    type = TimePeriod
    enable_objects = 'BCs::fix_x BCs::fix_y BCs::fix_z BCs::back_z'
    disable_objects = 'BCs::rotate_x BCs::rotate_y BCs::rotate_z'
    start_time = '0'
    end_time = '1'
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = FINITE
        add_variables = true
        use_finite_deform_jacobian = true
        use_automatic_differentiation = true
        generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_xz'
      [../]
    [../]
  [../]
[]


[Materials]
  [./stress]
    type = ADComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  [../]
  [./czm_mat]
    type = PureElasticTractionSeparation
    boundary = 'interface'
    normal_stiffness = 10000
    tangent_stiffness = 7000
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  # Executioner
  type = Transient
  solve_type = 'NEWTON'
  line_search = none
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-30
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 0.1
  end_time = 2
[]

[Outputs]
  exodus = true
  csv =true
[]

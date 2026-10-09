# An elastic block, fixed at its bottom face, whose top face is rotated rigidly by 90 degrees about
# the vertical axis through its center under large deformation kinematics. The rotation is
# prescribed either by RigidBodyDisplacementBC or, selected by 'BCs/active' in the test
# specification, by DisplacementAboutAxis, which gives the same finite rotation about one axis.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Mesh]
  [block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 4
    nz = 2
    ymax = 2
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = FINITE
        new_system = true
        add_variables = true
        formulation = TOTAL
      []
    []
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeStVenantKirchhoffStress
  []
[]

[Functions]
  [theta]
    type = ParsedFunction
    expression = 'pi / 2 * t'
  []
  [angle_degrees]
    type = ParsedFunction
    expression = '90 * t'
  []
[]

[BCs]
  active = 'fix_x fix_y fix_z rigid_x rigid_y rigid_z'
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []
  [rigid_x]
    type = RigidBodyDisplacementBC
    variable = disp_x
    component = x
    boundary = top
    reference_point = '0.5 2 0.5'
    rotations = '0 theta 0'
  []
  [rigid_y]
    type = RigidBodyDisplacementBC
    variable = disp_y
    component = y
    boundary = top
    reference_point = '0.5 2 0.5'
    rotations = '0 theta 0'
  []
  [rigid_z]
    type = RigidBodyDisplacementBC
    variable = disp_z
    component = z
    boundary = top
    reference_point = '0.5 2 0.5'
    rotations = '0 theta 0'
  []
  [axis_x]
    type = DisplacementAboutAxis
    variable = disp_x
    component = 0
    boundary = top
    function = angle_degrees
    angle_units = degrees
    axis_origin = '0.5 2 0.5'
    axis_direction = '0 1 0'
  []
  [axis_y]
    type = DisplacementAboutAxis
    variable = disp_y
    component = 1
    boundary = top
    function = angle_degrees
    angle_units = degrees
    axis_origin = '0.5 2 0.5'
    axis_direction = '0 1 0'
  []
  [axis_z]
    type = DisplacementAboutAxis
    variable = disp_z
    component = 2
    boundary = top
    function = angle_degrees
    angle_units = degrees
    axis_origin = '0.5 2 0.5'
    axis_direction = '0 1 0'
  []
[]

[Postprocessors]
  [disp_x_mid]
    type = PointValue
    variable = disp_x
    point = '1 1 1'
  []
  [disp_y_mid]
    type = PointValue
    variable = disp_y
    point = '1 1 1'
  []
  [disp_z_mid]
    type = PointValue
    variable = disp_z
    point = '1 1 1'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  dt = 0.25
  end_time = 1
[]

[Outputs]
  csv = true
[]

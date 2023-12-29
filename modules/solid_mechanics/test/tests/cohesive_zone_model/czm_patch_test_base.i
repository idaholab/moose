# Patch test for cohesive zone modeling to check the jacobian of cohesive kernels and materials.
# One test of this kind should be included when adding a new traction separation law.
# To preperly check the cohesive zone Jacobian, the cohesive stiffness should be low compared to the bulk stiffness.
# Quadratic convergence is always expected.

[Mesh]
  [./msh]
  type = FileMeshGenerator
  file = patch_mesh.e
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = msh
  []
  [./add_surfaces]
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
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = FINITE
        add_variables = true
        use_finite_deform_jacobian = true
        use_automatic_differentiation = true
      [../]
    [../]
  [../]
[]


[Functions]
  [./stretch]
    type = PiecewiseLinear
    x = '0 0.05'
    y = '0 0.1'
  [../]
[]

[Constraints]
  [x1]
    type = EqualValueBoundaryConstraint
    variable = disp_x
    secondary = 'x1'    # boundary
    penalty = 1e6
  []
  [y1]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    secondary = 'y1'    # boundary
    penalty = 1e6
  []
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = 'x0'
    variable = disp_x
  [../]
  [./fix_y]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = 'y0'
    variable = disp_y
  [../]
  [./fix_z]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = 'z0'
    variable = disp_z
  [../]
  [./back_z]
    type = FunctionDirichletBC
    boundary = 'z1'
    variable = disp_z
    use_displaced_mesh = true
    function = stretch
  [../]

  [./rotate_x]
    type = DisplacementAboutAxis
    boundary = 'x0 y0 z0 x1 y1 z1'
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
    boundary = 'x0 y0 z0 x1 y1 z1'
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
    boundary = 'x0 y0 z0 x1 y1 z1'
    function = '90.'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 1. 0.'
    component = 2
    variable = disp_z
    angular_velocity = true
  [../]
[]

[Controls]
  [./c1]
    type = TimePeriod
    enable_objects = 'BCs::fix_x BCs::fix_y BCs::fix_z BCs::back_z Constraints::x1 Constraints::y1'
    disable_objects = 'BCs::rotate_x BCs::rotate_y BCs::rotate_z'
    start_time = '0'
    end_time = '0.05'
  [../]
[]

[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm_ik]
    boundary = 'interface'
  [../]
[]

[Materials]
  [./stress]
    type = ADComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
  [../]
  [./czm_mat]
    boundary = 'interface'
  [../]
[]



[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  start_time = 0.0
  dt = 0.025
  end_time = 0.075
[]

[Postprocessors]
  [./nonlin]
    type = NumNonlinearIterations
  [../]
[]


[Outputs]
  csv = true
  exodus = true
[]

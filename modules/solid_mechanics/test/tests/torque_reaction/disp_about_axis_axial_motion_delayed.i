[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction=true
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = FINITE
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    add_variables = true
    decomposition_method = EigenSolution
    use_finite_deform_jacobian = true
  []
[]

[BCs]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]

# Because rotation is prescribed about the z axis, the
# DisplacementAboutAxis BC is only needed for the x and y
# displacements.
  [./top_x]
    type = DisplacementAboutAxis
    boundary = top
    function = 't'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 0
    variable = disp_x
  [../]

  [./top_y]
    type = DisplacementAboutAxis
    boundary = top
    function = 't'
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 1
    variable = disp_y
  [../]

  # DisplacementAboutAxis incremental
  [./top_x_rate]
    type = DisplacementAboutAxis
    boundary = top
    function = 1
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 0
    variable = disp_x
    angular_velocity = true
  [../]

  [./top_y_rate]
    type = DisplacementAboutAxis
    boundary = top
    function = 1
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 1
    variable = disp_y
    angular_velocity = true
  [../]
[]

  # Engage the incremental DisplacementAboutAxis after 30 seconds
[Controls]
  [./c1]
    type = TimePeriod
    enable_objects = 'BCs::top_x BCs::top_y'
    disable_objects = 'BCs::top_x_rate BCs::top_y_rate'
    start_time = '0'
    end_time = '30'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Postprocessors]
  [./disp_x_5]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 5
  [../]
  [./disp_y_5]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 5
  [../]
  [./disp_x_6]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 6
  [../]
  [./disp_y_6]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 6
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 30
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-9
  l_tol = 1e-8

  start_time = 0.0
  dt = 2
  dtmin = 2 # die instead of cutting the timestep

  end_time = 90
[]

[Outputs]
  file_base = disp_about_axis_axial_motion_delayed_out
  csv = true
[]

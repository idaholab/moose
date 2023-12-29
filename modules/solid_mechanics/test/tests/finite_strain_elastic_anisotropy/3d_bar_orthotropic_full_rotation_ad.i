[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 2
    nx = 1
    ny = 1
    nz = 1
    elem_type = HEX8
  []
  [corner]
    type = ExtraNodesetGenerator
    new_boundary = 101
    coord = '0 0 0'
    input = generated_mesh
  []
  [side]
    type = ExtraNodesetGenerator
    new_boundary = 102
    coord = '2 0 0'
    input = corner
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    use_finite_deform_jacobian = true
    volumetric_locking_correction = false
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_xz'
    use_automatic_differentiation = true
  []
[]

[Materials]
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '2.0e3 2.0e5 2.0e3 0.71428571e3 0.71428571e3 0.71428571e3 0.4 0.2 0.004 0.004 0.2 0.4'
  []
[]

[BCs]
  [fix_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []

  [rot_y]
    type = DisplacementAboutAxis
    boundary = bottom
    function = t
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 1
    variable = disp_y
  []
  #
  [rot_x]
    type = DisplacementAboutAxis
    boundary = bottom
    function = t
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 0
    variable = disp_x
  []

  [rot_y90]
    type = DisplacementAboutAxis
    boundary = bottom
    function = 360
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 1
    variable = disp_y
  []
  #
  [rot_x90]
    type = DisplacementAboutAxis
    boundary = bottom
    function = 360
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 0
    variable = disp_x
  []

  [press]
    boundary = top
    function = '-1.0*(t-360)*10.0'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
    type = Pressure
    variable = disp_y
  []

[]

[Controls]
  [c1]
    type = TimePeriod
    enable_objects = 'BCs::rot_x BCs::rot_y'
    disable_objects = 'BCs::rot_x90 BCs::rot_y90 BCs::press'
    start_time = '0'
    end_time = '360'
  []
  [c190plus]
    type = TimePeriod
    enable_objects = 'BCs::rot_x90 BCs::rot_y90 BCs::press'
    disable_objects = 'BCs::rot_x BCs::rot_y '
    start_time = '360'
    end_time = '660'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-08
  nl_max_its = 50

  l_tol = 1e-4
  l_max_its = 50
  start_time = 0.0

  dt = 5
  dtmin = 5

  num_steps = 132
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

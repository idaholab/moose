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
    volumetric_locking_correction = true
    use_automatic_differentiation = true
    generate_output = 'elastic_strain_xx stress_xx creep_strain_xx'
  []
[]

[Materials]
  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_creep"
    max_iterations = 50
    absolute_tolerance = 1e-18
  []
  [hill_tensor]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.5 1.0 0.5 1.5 1.5 1.5"
    use_large_rotation = true
  []
  [trial_creep]
    type = ADHillCreepStressUpdate
    coefficient = 5e-14
    n_exponent = 10
    m_exponent = 0
    activation_energy = 0
    max_inelastic_increment = 0.00003
    absolute_tolerance = 1e-18
    relative_tolerance = 1e-18
    # Force it to not use integration error
    max_integration_error = 100.0
    use_transformation = true
  []
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 500
    poissons_ratio = 0.0
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
    function = 90
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
    function = 90
    angle_units = degrees
    axis_origin = '0. 0. 0.'
    axis_direction = '0. 0. 1.'
    component = 0
    variable = disp_x
  []

  [press]
    boundary = top
    function = '-1.0*(t-90)*0.1'
    use_displaced_mesh = true
    displacements = 'disp_x disp_y disp_z'
    type = Pressure
    variable = disp_x
  []
[]

[Postprocessors]
  [creep_strain_xx]
    type = ADElementAverageMaterialProperty
    mat_prop = creep_strain_xx
  []
[]

[Controls]
  [c1]
    type = TimePeriod
    enable_objects = 'BCs::rot_x BCs::rot_y'
    disable_objects = 'BCs::rot_x90 BCs::rot_y90 BCs::press'
    start_time = '0'
    end_time = '90'
  []
  [c190plus]
    type = TimePeriod
    enable_objects = 'BCs::rot_x90 BCs::rot_y90 BCs::press'
    disable_objects = 'BCs::rot_x BCs::rot_y '
    start_time = '90'
    end_time = '390'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  nl_max_its = 50

  automatic_scaling = true
  l_tol = 1e-4
  l_max_its = 50
  start_time = 0.0

  dt = 0.1
  dtmin = 0.1

  num_steps = 1200
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = false
  csv = true
[]

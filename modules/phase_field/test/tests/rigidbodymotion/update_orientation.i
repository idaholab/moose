# test file for applyting advection term and observing rigid body motion of grains
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 15
  nz = 0
  xmax = 50
  ymax = 25
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
  [./eta]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
    coupled_variables = eta
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  [./motion]
    type = MultiGrainRigidBodyMotion
    variable = w
    c = c
    v = eta
    grain_tracker_object = grain_center
    grain_force = grain_force
    grain_volumes = grain_volumes
  [../]
  [./eta_dot]
    type = TimeDerivative
    variable = eta
  [../]
  [./vadv_eta]
    type = SingleGrainRigidBodyMotion
    variable = eta
    c = c
    v = eta
    grain_tracker_object = grain_center
    grain_force = grain_force
    grain_volumes = grain_volumes
  [../]
  [./acint_eta]
    type = ACInterface
    variable = eta
    mob_name = M
    coupled_variables = c
    kappa_name = kappa_eta
  [../]
  [./acbulk_eta]
    type = AllenCahn
    variable = eta
    mob_name = M
    f_name = F
    coupled_variables = c
  [../]
[]

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names = 'M    kappa_c  kappa_eta'
    prop_values = '5.0  2.0      0.1'
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    coupled_variables = 'c eta'
    constant_names = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    expression = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2+(c-eta)^2
    derivative_order = 2
  [../]
[]

[AuxVariables]
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./var_indices]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./centroids]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vadv_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vadv_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./angle_initial]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./euler_angle]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_center
    field_display = UNIQUE_REGION
    execute_on = timestep_begin
  [../]
  [./var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_center
    field_display = VARIABLE_COLORING
    execute_on = timestep_begin
  [../]
  [./centroids]
    type = FeatureFloodCountAux
    variable = centroids
    execute_on = timestep_begin
    field_display = CENTROID
    flood_counter = grain_center
  [../]
  [./vadv_x]
    type = GrainAdvectionAux
    grain_force = grain_force
    grain_volumes = grain_volumes
    grain_tracker_object = grain_center
    execute_on = timestep_begin
    component = x
    variable = vadv_x
  [../]
  [./vadv_y]
    type = GrainAdvectionAux
    grain_force = grain_force
    grain_volumes = grain_volumes
    grain_tracker_object = grain_center
    execute_on = timestep_begin
    component = y
    variable = vadv_y
  [../]
  [./angle_initial]
    type = OutputEulerAngles
    variable = angle_initial
    euler_angle_provider = euler_angle_initial
    grain_tracker = grain_center
    output_euler_angle = phi2
    execute_on = timestep_begin
  [../]
  [./angle]
    type = OutputEulerAngles
    variable = euler_angle
    euler_angle_provider = euler_angle
    grain_tracker = grain_center
    output_euler_angle = phi2
    execute_on = timestep_begin
  [../]
[]


[VectorPostprocessors]
  [./forces]
    type = GrainForcesPostprocessor
    grain_force = grain_force
  [../]
  [./grain_volumes]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = grain_center
    execute_on = 'initial timestep_begin'
  [../]
[]

[UserObjects]
  [./grain_center]
    type = GrainTracker
    variable = eta
    outputs = none
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_begin'
  [../]
  [./grain_force]
    type = ConstantGrainForceAndTorque
    execute_on = 'initial timestep_begin linear nonlinear'
    force = '0.5 0.0 0.0 '
    torque = '0.0 0.0 10.0'
  [../]
  [./euler_angle_initial]
    type = RandomEulerAngleProvider
    grain_tracker_object = grain_center
    execute_on = 'initial timestep_begin'
  [../]
  [./euler_angle]
    type = EulerAngleUpdater
    grain_tracker_object = grain_center
    euler_angle_provider = euler_angle_initial
    grain_torques_object = grain_force
    grain_volumes = grain_volumes
    execute_on = timestep_begin
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31   preonly   lu      1'
  nl_max_its = 30
  l_max_its = 30
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  dt = 0.2
  num_steps = 5
[]

[Outputs]
  exodus = true
[]

[ICs]
  [./rect_c]
    y2 = 20.0
    y1 = 5.0
    inside = 1.0
    x2 = 30.0
    variable = c
    x1 = 10.0
    type = BoundingBoxIC
  [../]
  [./rect_eta]
    y2 = 20.0
    y1 = 5.0
    inside = 1.0
    x2 = 30.0
    variable = eta
    x1 = 10.0
    type = BoundingBoxIC
  [../]
[]

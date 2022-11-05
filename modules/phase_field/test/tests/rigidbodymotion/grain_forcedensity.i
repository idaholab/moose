# test file for showing reaction forces between particles
[GlobalParams]
  var_name_base = eta
  op_num = 2
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 5
  nz = 0
  xmax = 50
  ymax = 25
  zmax = 0
  elem_type = QUAD4
  uniform_refine = 1
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
  [./eta0]
  [../]
  [./eta1]
  [../]
[]

[Kernels]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    coupled_variables = 'eta0 eta1'
    w = w
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
    v = 'eta0 eta1'
    grain_force = grain_force
    grain_tracker_object = grain_center
    grain_volumes = grain_volumes
  [../]
  [./eta0_dot]
    type = TimeDerivative
    variable = eta0
  [../]
  [./vadv_eta]
    type = SingleGrainRigidBodyMotion
    variable = eta0
    c = c
    v = 'eta0 eta1'
    grain_force = grain_force
    grain_tracker_object = grain_center
    grain_volumes = grain_volumes
    op_index = 0
  [../]
  [./acint_eta0]
    type = ACInterface
    variable = eta0
    mob_name = M
    #coupled_variables = c
    kappa_name = kappa_eta
  [../]
  [./acbulk_eta0]
    type = AllenCahn
    variable = eta0
    mob_name = M
    f_name = F
    coupled_variables = 'c eta1'
  [../]
  [./eta1_dot]
    type = TimeDerivative
    variable = eta1
  [../]
  [./vadv_eta1]
    type = SingleGrainRigidBodyMotion
    variable = eta1
    c = c
    v = 'eta0 eta1'
    op_index = 1
    grain_force = grain_force
    grain_tracker_object = grain_center
    grain_volumes = grain_volumes
  [../]
  [./acint_eta1]
    type = ACInterface
    variable = eta1
    mob_name = M
    #coupled_variables = c
    kappa_name = kappa_eta
  [../]
  [./acbulk_eta1]
    type = AllenCahn
    variable = eta1
    mob_name = M
    f_name = F
    coupled_variables = 'c eta0'
  [../]
[]

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names = 'M    kappa_c  kappa_eta'
    prop_values = '1.0  0.5      0.5'
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'c eta0 eta1'
    constant_names = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    expression = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2+eta0*(1-eta0)*c+eta1*(1-eta1)*c
    derivative_order = 2
  [../]
  [./force_density]
    type = ForceDensityMaterial
    c = c
    etas ='eta0 eta1'
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
  [./df00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./df01]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./df10]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./df11]
    order = CONSTANT
    family = MONOMIAL
  [../]
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
[]

[AuxKernels]
  [./bnds]
    type = BndsCalcAux
    variable = bnds
    var_name_base = eta
    op_num = 2
    v = 'eta0 eta1'
  [../]
  [./df01]
    type = MaterialStdVectorRealGradientAux
    variable = df01
    index = 0
    component = 1
    property = force_density
  [../]
  [./df11]
    type = MaterialStdVectorRealGradientAux
    variable = df11
    index = 1
    component = 1
    property = force_density
  [../]
  [./df00]
    type = MaterialStdVectorRealGradientAux
    variable = df00
    index = 0
    component = 0
    property = force_density
  [../]
  [./df10]
    type = MaterialStdVectorRealGradientAux
    variable = df10
    index = 1
    component = 0
    property = force_density
  [../]
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
[]

[ICs]
  [./ic_eta0]
    int_width = 1.0
    x1 = 20.0
    y1 = 0.0
    radius = 14.0
    outvalue = 0.0
    variable = eta0
    invalue = 1.0
    type = SmoothCircleIC
  [../]
  [./IC_eta1]
    int_width = 1.0
    x1 = 30.0
    y1 = 25.0
    radius = 14.0
    outvalue = 0.0
    variable = eta1
    invalue = 1.0
    type = SmoothCircleIC
  [../]
  [./ic_c]
    type = SpecifiedSmoothCircleIC
    invalue = 1.0
    outvalue = 0.1
    int_width = 1.0
    x_positions = '20.0 30.0 '
    z_positions = '0.0 0.0 '
    y_positions = '0.0 25.0 '
    radii = '14.0 14.0'
    3D_spheres = false
    variable = c
    block = 0
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
    outputs = none
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_begin'
  [../]
  [./grain_force]
    type = ComputeGrainForceAndTorque
    execute_on = 'linear nonlinear'
    grain_data = grain_center
    force_density = force_density
    c = c
    etas = 'eta0 eta1'
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
  l_max_its = 30
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  num_steps = 1
  dt = 0.1
[]

[Outputs]
  exodus = true
  csv = true
[]

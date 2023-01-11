# example showing grain motion due to applied force density on grains
[GlobalParams]
  var_name_base = eta
  op_num = 4
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 80
  ny = 40
  nz = 0
  xmin = 0.0
  xmax = 40.0
  ymin = 0.0
  ymax = 20.0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./c]
  [../]
  [./w]
  [../]
  [./PolycrystalVariables]
  [../]
[]

[Kernels]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
    coupled_variables = 'eta0 eta1 eta2 eta3'
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
    v = 'eta0 eta1 eta2 eta3'
    grain_tracker_object = grain_center
    grain_force = grain_force
    grain_volumes = grain_volumes
  [../]

  [./RigidBodyMultiKernel]
    # Creates all of the necessary Allen Cahn kernels automatically
    c = c
    f_name = F
    mob_name = L
    kappa_name = kappa_eta
    grain_force = grain_force
    grain_volumes = grain_volumes
    grain_tracker_object = grain_center
  [../]
[]

[Functions]
  [./load_x]
    # Defines the force on the grains in the x-direction
    type = ParsedFunction
    expression = 0.005*cos(x*pi/600)
  [../]
  [./load_y]
    # Defines the force on the grains in the y-direction
    type = ConstantFunction
    value = 0.002
  [../]
[]

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names = 'M    L kappa_c  kappa_eta'
    prop_values = '4.5 60  250      4000'
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    #coupled_variables = 'c eta0 eta1 eta2 eta3'
    #constant_names = 'barr_height  cv_eq'
    #constant_expressions = '0.1          1.0e-2'
    #function = '16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2
    #           +eta0*(1-eta0)*c+eta1*(1-eta1)*c
    #           +eta2*(1-eta2)*c+eta3*(1-eta3)*c'
    constant_names = 'A B'
    constant_expressions = '450 1.5'
    coupled_variables = 'c eta0 eta1 eta2 eta3' #Must be changed as op_num changes. Copy/paste from line 4
    expression = 'A*c^2*(1-c)^2+B*(c^2+6*(1-c)*(eta0^2+eta1^2+eta2^2+eta3^2)
                -4*(2-c)*(eta0^3+eta1^3+eta2^3+eta3^3)
                +3*(eta0^2+eta1^2+eta2^2+eta3^2)^2)'
    derivative_order = 2
  [../]
  #[./force_density]
  #  type = ForceDensityMaterial
  #  c = c
  #  etas = 'eta0 eta1 eta2 eta3'
  #[../]
  [./force_density]
    type = ExternalForceDensityMaterial
    c = c
    k = 10.0
    etas = 'eta0 eta1 eta2 eta3'
    force_x = load_x
    force_y = load_y
  [../]
[]

[AuxVariables]
  [./bnds]
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
    #var_name_base = eta
    #op_num = 4.0
    v = 'eta0 eta1 eta2 eta3'
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
  [./ic_eta1]
    x_positions = '32.5 24.0'
    int_width = 1.0
    z_positions = '0 0'
    y_positions = '6.0 14.0'
    radii = '4.0 4.0'
    3D_spheres = false
    outvalue = 0
    variable = eta1
    invalue = 1
    type = SpecifiedSmoothCircleIC
    block = 0
  [../]
  [./multip]
    x_positions = '5.5 15.5 24.0 32.5 7.0 15.5 24.0 32.5'
    int_width = 1.0
    z_positions = '0 0'
    y_positions = '6.0 6.0 6.0 6.0 14.5 14.5 14.0 14.5'
    radii = '4.0 4.0 4.0 4.0 4.0 4.0 4.0 4.0'
    3D_spheres = false
    outvalue = 0.001
    variable = c
    invalue = 0.999
    type = SpecifiedSmoothCircleIC
    block = 0
  [../]
  [./ic_eta0]
    x_positions = '5.5 15.5'
    int_width = 1.0
    z_positions = '0 0'
    y_positions = '6.0 6.0'
    radii = '4.0 4.0'
    3D_spheres = false
    outvalue = 0.0
    variable = eta0
    invalue = 1.0
    type = SpecifiedSmoothCircleIC
    block = 0
  [../]
  [./ic_eta2]
    x_positions = '24.0 7.0'
    int_width = 1.0
    z_positions = '0 0'
    y_positions = '6.0 14.5 '
    radii = '4.0 4.0 '
    3D_spheres = false
    outvalue = 0.0
    variable = eta2
    invalue = 1.0
    type = SpecifiedSmoothCircleIC
    block = 0
  [../]
  [./ic_eta3]
    x_positions = '15.5 32.5'
    int_width = 1.0
    z_positions = '0 0'
    y_positions = '14.5 14.5'
    radii = '4.0 4.0'
    3D_spheres = false
    outvalue = 0.0
    variable = eta3
    invalue = 1.0
    type = SpecifiedSmoothCircleIC
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
    type = ComputeExternalGrainForceAndTorque
    c = c
    grain_data = grain_center
    force_density = force_density_ext
    etas = 'eta0 eta1 eta2 eta3'
    execute_on = 'initial linear nonlinear'
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
  num_steps = 20
  dt = 0.01
[]

[Outputs]
  exodus = true
[]

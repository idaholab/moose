# test file for showing grain motion due to applied force density on grains

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 10
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
    [./InitialCondition]
      type = SpecifiedSmoothCircleIC
      invalue = 1.0
      outvalue = 0.1
      int_width = 6.0
      x_positions = '20.0 30.0 '
      z_positions = '0.0 0.0 '
      y_positions = '0.0 25.0 '
      radii = '14.0 14.0'
      3D_spheres = false
      variable = c
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./load]
    type = ConstantFunction
    value = 0.01
  [../]
[]

[Kernels]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
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
    f_name = F
    args = c
    constant_names = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    function = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2
    derivative_order = 2
  [../]
  [./force_density]
    type = ExternalForceDensityMaterial
    c = c
    etas = 'eta0 eta1'
    k = 1.0
    force_y = load
  [../]
  [./advection_vel]
    type = GrainAdvectionVelocity
    grain_force = grain_force
    etas = 'eta0 eta1'
    c = c
    grain_data = grain_center
  [../]
[]

[AuxVariables]
  [./eta0]
  [../]
  [./eta1]
  [../]
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
  [./vadv00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vadv01]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vadv10]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vadv11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vadv0_div]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vadv1_div]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./bnds]
    type = BndsCalcAux
    variable = bnds
    var_name_base = eta
    op_num = 2.0
    v = 'eta0 eta1'
  [../]
  [./df01]
    type = MaterialStdVectorRealGradientAux
    variable = df01
    component = 1
    property = force_density_ext
  [../]
  [./df11]
    type = MaterialStdVectorRealGradientAux
    variable = df11
    index = 1
    component = 1
    property = force_density_ext
  [../]
  [./df00]
    type = MaterialStdVectorRealGradientAux
    variable = df00
    property = force_density_ext
  [../]
  [./df10]
    type = MaterialStdVectorRealGradientAux
    variable = df10
    index = 1
    property = force_density_ext
  [../]
  [./vadv00]
    type = MaterialStdVectorRealGradientAux
    variable = vadv00
    property = advection_velocity
  [../]
  [./vadv01]
    type = MaterialStdVectorRealGradientAux
    variable = vadv01
    property = advection_velocity
    component = 1
  [../]
  [./vadv10]
    type = MaterialStdVectorRealGradientAux
    variable = vadv10
    index = 1
    property = advection_velocity
  [../]
  [./vadv11]
    type = MaterialStdVectorRealGradientAux
    variable = vadv11
    property = advection_velocity
    index = 1
    component = 1
  [../]
  [./vadv0_div]
    type = MaterialStdVectorAux
    variable = vadv0_div
    property = advection_velocity_divergence
  [../]
  [./vadv1_div]
    type = MaterialStdVectorAux
    variable = vadv1_div
    property = advection_velocity_divergence
    index = 1
  [../]
[]

[ICs]
  [./ic_eta0]
    int_width = 6.0
    x1 = 20.0
    y1 = 0.0
    radius = 14.0
    outvalue = 0.0
    variable = eta0
    invalue = 1.0
    type = SmoothCircleIC
  [../]
  [./IC_eta1]
    int_width = 6.0
    x1 = 30.0
    y1 = 25.0
    radius = 14.0
    outvalue = 0.0
    variable = eta1
    invalue = 1.0
    type = SmoothCircleIC
  [../]
[]

[VectorPostprocessors]
  [./centers]
    type = GrainCentersPostprocessor
    grain_data = grain_center
  [../]
  [./forces]
    type = GrainForcesPostprocessor
    grain_force = grain_force
  [../]
[]

[UserObjects]
  [./grain_center]
    type = ComputeGrainCenterUserObject
    etas = 'eta0 eta1'
    execute_on = 'initial timestep_end'
  [../]
  [./grain_force]
    type = ComputeGrainForceAndTorque
    execute_on = 'initial timestep_end'
    grain_data = grain_center
    c = c
    force_density = force_density_ext
  [../]
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
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

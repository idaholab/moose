# test file for applyting constant forces and torques on grains
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 25
  ymin = 0
  ymax = 30
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SpecifiedSmoothCircleIC
      x_positions = '10.0 20.0'
      y_positions = '15.0 17.0'
      z_positions = '0.0 0.0'
      radii = '6.0 6.0'
      invalue = 1.0
      outvalue = 0.0
      int_width = 3.0
    [../]
  [../]
  [./w]
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
    prop_names  = 'M kappa_c'
    prop_values = '1e-3 0.1'
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    f_name = F
    args = 'c eta0 eta1'
    constant_names = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    function = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2+(c-eta0)^2+(c-eta1)^2
    derivative_order = 2
  [../]
  [./force_density]
    type = ForceDensityMaterial
    block = 0
    c = c
    etas = 'eta0 eta1'
  [../]
  [./advection_vel]
    type = GrainAdvectionVelocity
    block = 0
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
[]

[ICs]
  [./eta0]
    type = SmoothCircleIC
    x1 = 10.0
    y1 = 15.0
    radius = 6.0
    invalue = 1.0
    outvalue = 0.0
    int_width = 3.0
    variable = eta0
  [../]
  [./eta1]
    type = SmoothCircleIC
    x1 = 20.0
    y1 = 17.0
    radius = 6.0
    invalue = 1.0
    outvalue = 0.0
    int_width = 3.0
    variable = eta1
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
    execute_on = 'initial linear'
  [../]
  #[./grain_force]
  #  type = ConstantGrainForceAndTorque
  #  execute_on = 'initial linear'
  #  force = '0.2 0.0 0.0 '
  #  torque = '0.0 0.0 5.0 '
  #[../]
  [./grain_force]
    type = ComputeGrainForceAndTorque
    execute_on = 'initial linear'
    grain_data = grain_center
    force_density = force_density
    c = c
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
  solve_type = NEWTON
  #petsc_options = '-snes_test_display'
  #petsc_options_iname = '-snes_type'
  #petsc_options_value = 'test'
  #petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  #petsc_options_value = 'asm         31   preonly   lu      1'
  l_max_its = 30
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  num_steps = 1
  dt = 1
[]

[Outputs]
  exodus = true
  csv = true
[]

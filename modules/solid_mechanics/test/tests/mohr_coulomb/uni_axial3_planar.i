# same as uni_axial2 but with planar mohr-coulomb
[Mesh]
  type = FileMesh
  file = quarter_hole.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [SolidMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[BCs]
  [./zmin_zzero]
    type = DirichletBC
    variable = disp_z
    boundary = 'zmin'
    value = '0'
  [../]
  [./xmin_xzero]
    type = DirichletBC
    variable = disp_x
    boundary = 'xmin'
    value = '0'
  [../]
  [./ymin_yzero]
    type = DirichletBC
    variable = disp_y
    boundary = 'ymin'
    value = '0'
  [../]
  [./ymax_disp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'ymax'
    function = '-1E-4*t'
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mc_int]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./mc_int_auxk]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = mc_int
  [../]
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_yield_function
    variable = yield_fcn
  [../]
[]

[Postprocessors]
  [./s_xx]
    type = PointValue
    point = '0.005 0.02 0.002'
    variable = stress_xx
  [../]
  [./s_xy]
    type = PointValue
    point = '0.005 0.02 0.002'
    variable = stress_xy
  [../]
  [./s_xz]
    type = PointValue
    point = '0.005 0.02 0.002'
    variable = stress_xz
  [../]
  [./s_yy]
    type = PointValue
    point = '0.005 0.02 0.002'
    variable = stress_yy
  [../]
  [./s_yz]
    type = PointValue
    point = '0.005 0.02 0.002'
    variable = stress_yz
  [../]
  [./s_zz]
    type = PointValue
    point = '0.005 0.02 0.002'
    variable = stress_zz
  [../]
  [./f]
    type = PointValue
    point = '0.005 0.02 0.002'
    variable = yield_fcn
  [../]
[]

[UserObjects]
  [./coh]
    type = SolidMechanicsHardeningConstant
    value = 1E7
  [../]
  [./fric]
    type = SolidMechanicsHardeningConstant
    value = 40
    convert_to_radians = true
  [../]
  [./dil]
    type = SolidMechanicsHardeningConstant
    value = 40
    convert_to_radians = true
  [../]

  [./mc]
    type = SolidMechanicsPlasticMohrCoulombMulti
    cohesion = coh
    friction_angle = fric
    dilation_angle = dil
    yield_function_tolerance = 1.0 # THIS IS HIGHER THAN THE SMOOTH CASE TO AVOID PRECISION-LOSS PROBLEMS!
    shift =  1.0
    use_custom_returnMap = false
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 1
    fill_method = symmetric_isotropic
    C_ijkl = '0 5E9' # young = 10Gpa, poisson = 0.0
  [../]
  [./strain]
    type = ComputeIncrementalStrain
    block = 1
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 1
    ep_plastic_tolerance = 1E-9
    plastic_models = mc
    max_NR_iterations = 100
    deactivation_scheme = 'safe'
    min_stepsize = 1
    max_stepsize_for_dumb = 1
    debug_fspb = crash
  [../]
[]

# Preconditioning and Executioner options kindly provided by Andrea
[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]


[Executioner]
  end_time = 1.05
  dt = 0.1
  solve_type = NEWTON
  type = Transient
[]


[Outputs]
  file_base = uni_axial3_planar
  [./exodus]
    type = Exodus
    hide = 'stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz yield_fcn s_xx s_xy s_xz s_yy s_yz s_zz f'
  [../]
  [./csv]
    type = CSV
    time_step_interval = 1
  [../]
[]

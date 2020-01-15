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
  [./TensorMechanics]
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
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 10E6
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 40
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 40
    convert_to_radians = true
  [../]
  [./mc]
    type = TensorMechanicsPlasticMohrCoulomb
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    mc_tip_smoother = 0.01E6
    mc_edge_smoother = 29
    yield_function_tolerance = 1E-5
    internal_constraint_tolerance = 1E-11
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
    type = ComputeIncrementalSmallStrain
    block = 1
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 1
    ep_plastic_tolerance = 1E-11
    plastic_models = mc
    max_NR_iterations = 1000
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

  nl_abs_tol = 1E-10
  nl_rel_tol = 1E-12
  l_tol = 1E-2
  l_max_its = 50
  nl_max_its = 400

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
[]


[Outputs]
  file_base = uni_axial3
  exodus = true
  [./csv]
    type = CSV
    [../]
[]

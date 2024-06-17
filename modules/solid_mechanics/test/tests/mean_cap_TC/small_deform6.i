# apply nonuniform stretch in x, y and z directions using
# Lame lambda = 0.7E7, Lame mu = 1.0E7,
# trial_stress(0, 0) = 2.9
# trial_stress(1, 1) = 10.9
# trial_stress(2, 2) = 14.9
# With tensile_strength = 2, decaying to zero at internal parameter = 4E-7
# via a Cubic, the algorithm should return to:
# internal parameter = 2.26829E-7
# trace(stress) = 0.799989 = tensile_strength
# stress(0, 0) = -6.4
# stress(1, 1) = 1.6
# stress(2, 2) = 5.6

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
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
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '-1E-7*x'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '3E-7*y'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '5E-7*z'
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
  [./f]
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
  [./f]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_yield_function
    variable = f
  [../]
[]

[Postprocessors]
  [./s_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./s_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./s_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./s_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./f]
    type = PointValue
    point = '0 0 0'
    variable = f
  [../]
[]

[UserObjects]
  [./tensile_strength]
    type = SolidMechanicsHardeningCubic
    value_0 = 2
    value_residual = 0
    internal_limit = 4E-7
  [../]
  [./compressive_strength]
    type = SolidMechanicsHardeningCubic
    value_0 = -1
    value_residual = 0
    internal_limit = 1E-8
  [../]
  [./cap]
    type = SolidMechanicsPlasticMeanCapTC
    tensile_strength = tensile_strength
    compressive_strength = compressive_strength
    yield_function_tolerance = 1E-5
    internal_constraint_tolerance = 1E-11
    use_custom_returnMap = true
    use_custom_cto = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0.7E7 1E7'
  [../]
  [./strain]
    type = ComputeIncrementalStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./mean_cap]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-11
    plastic_models = cap
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform6
  exodus = false
  [./csv]
    type = CSV
  [../]
[]

# UserObject Orthotropic test, with constant hardening.
# Linear strain is applied in the x and y direction.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin =  -.5
  xmax = .5
  ymin = -.5
  ymax = .5
  zmin = -.5
  zmax = .5
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_xz'
  [../]
[]

[BCs]
  [./xdisp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = '0.005*t'
  [../]
  [./ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top'
    function = '0.005*t'
  [../]
  [./yfix]
    type = DirichletBC
    variable = disp_y
    #boundary = 'bottom top'
    boundary = 'bottom'
    value = 0
  [../]
  [./xfix]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
  [./zfix]
    type = DirichletBC
    variable = disp_z
    #boundary = 'front back'
    boundary = 'back'
    value = 0
  [../]
[]

[AuxVariables]
  [./plastic_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sdev]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sdet]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./plastic_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_xx
    index_i = 0
    index_j = 0
  [../]
  [./plastic_xy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_xy
    index_i = 0
    index_j = 1
  [../]
  [./plastic_xz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_xz
    index_i = 0
    index_j = 2
  [../]
  [./plastic_yy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_yy
    index_i = 1
    index_j = 1
  [../]
  [./plastic_yz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_yz
    index_i = 1
    index_j = 2
  [../]
  [./plastic_zz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_zz
    index_i = 2
    index_j = 2
  [../]
  [./f]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_yield_function
    variable = f
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./intnl]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = intnl
  [../]
  [./sdev]
    type = RankTwoScalarAux
    variable = sdev
    rank_two_tensor = stress
    scalar_type = VonMisesStress
  [../]
[]

[Postprocessors]
  [./sdev]
    type = PointValue
    point = '0 0 0'
    variable = sdev
  [../]
  [./s_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./p_xx]
    type = PointValue
    point = '0 0 0'
    variable = plastic_xx
  [../]
  [./s_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./p_xy]
    type = PointValue
    point = '0 0 0'
    variable = plastic_xy
  [../]
  [./p_xz]
    type = PointValue
    point = '0 0 0'
    variable = plastic_xz
  [../]
  [./p_yz]
    type = PointValue
    point = '0 0 0'
    variable = plastic_yz
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
  [./p_yy]
    type = PointValue
    point = '0 0 0'
    variable = plastic_yy
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
  [./p_zz]
    type = PointValue
    point = '0 0 0'
    variable = plastic_zz
  [../]
  [./intnl]
    type = PointValue
    point = '0 0 0'
    variable = intnl
  [../]
[]

[UserObjects]
  [./str]
    type = TensorMechanicsHardeningConstant
    value = 300
  [../]
  [./Orthotropic]
    type = TensorMechanicsPlasticOrthotropic
    b = -0.2
    c1 = '1 1 1 1 1 1'
    c2 = '1 1 1 1 1 1'
    associative = true
    yield_strength = str
    yield_function_tolerance = 1e-5
    internal_constraint_tolerance = 1e-9
    use_custom_returnMap = false
    use_custom_cto = false
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '121e3 80e3'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1e-9
    plastic_models = Orthotropic
    debug_fspb = crash
    tangent_operator = elastic
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  num_steps = 3
  dt = .5
  type = Transient

  nl_rel_tol = 1e-6
  nl_max_its = 10
  l_tol = 1e-4
  l_max_its = 50

  solve_type = PJFNK
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]


[Outputs]
  perf_graph = false
  csv = true
[]

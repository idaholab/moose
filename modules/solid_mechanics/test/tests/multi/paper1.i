# This runs the models mentioned in the first example of the Multi-Surface paper
#
# Plasticity models:
# SimpleTester with a = 1 and b = 0 and strength = 1E9  (only does elasticity)
# SimpleTester with a = 1 and b = 0 and strength = 0
# SimpleTester with a = 1 and b = 0 and strength = 1E-3
#
# Lame lambda = 0 (Poisson=0).  Lame mu = 0.5E6
#
# A line of elements is perturbed randomly, and return to the yield surface at each quadpoint is checked

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 125
  nz = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 125
  zmin = 0
  zmax = 1
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


[ICs]
  [./x]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_x
  [../]
  [./y]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_y
  [../]
  [./z]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_z
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '0'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '0'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '0'
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
  [./linesearch]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ld]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./constr_added]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
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
  [./linesearch]
    type = MaterialRealAux
    property = plastic_linesearch_needed
    variable = linesearch
  [../]
  [./ld]
    type = MaterialRealAux
    property = plastic_linear_dependence_encountered
    variable = ld
  [../]
  [./constr_added]
    type = MaterialRealAux
    property = plastic_constraints_added
    variable = constr_added
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
[]

[Postprocessors]
  [./max_iter]
    type = ElementExtremeValue
    variable = iter
    outputs = console
  [../]
  [./av_linesearch]
    type = ElementAverageValue
    variable = linesearch
    outputs = console
  [../]
  [./av_ld]
    type = ElementAverageValue
    variable = ld
    outputs = console
  [../]
  [./av_constr_added]
    type = ElementAverageValue
    variable = constr_added
    outputs = console
  [../]
  [./av_iter]
    type = ElementAverageValue
    variable = iter
    outputs = console
  [../]
[]


[UserObjects]
  [./simple0]
    type = TensorMechanicsPlasticSimpleTester
    a = 1
    b = 0
    strength = 1E9
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
  [./simple1]
    type = TensorMechanicsPlasticSimpleTester
    a = 1
    b = 0
    strength = 0
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
  [./simple2]
    type = TensorMechanicsPlasticSimpleTester
    a = 1
    b = 0
    strength = 1E-3
    yield_function_tolerance = 1.0E-6
    internal_constraint_tolerance = 1.0E-6
  [../]
[]

[Materials]
  active = 'elasticity_tensor strain single'
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 0.5E6'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./elastic_model]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-7
    plastic_models = 'simple0'
  [../]
  [./single]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-7
    plastic_models = 'simple1'
  [../]
  [./double]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-7
    plastic_models = 'simple1 simple2'
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = paper1
  exodus = false
  csv = true
[]

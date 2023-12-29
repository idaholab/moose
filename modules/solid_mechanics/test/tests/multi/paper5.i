# This runs the J2+cap+hardening example model described in the 'MultiSurface' plasticity paper
#
# Plasticity models:
# J2 with strength = 20MPa to 10MPa in 100% strain
# Compressive cap with strength = 15MPa to 5MPa in 100% strain
#
# Lame lambda = 1.2GPa.  Lame mu = 1.2GPa (Young = 3GPa, poisson = 0.25)
#
# A line of elements is perturbed randomly, and return to the yield surface at each quadpoint is checked

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1000
  ny = 125
  nz = 1
  xmin = 0
  xmax = 1000
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
  [./intnl0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f1]
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
  [./f0]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f0
  [../]
  [./f1]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f1
  [../]
  [./intnl0]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = intnl0
  [../]
  [./intnl1]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 1
    variable = intnl1
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
  [./max_f0]
    type = ElementExtremeValue
    variable = f0
    outputs = console
  [../]
  [./max_f1]
    type = ElementExtremeValue
    variable = f1
    outputs = console
  [../]
  [./max_iter]
    type = ElementExtremeValue
    variable = iter
    outputs = console
  [../]
  [./av_iter]
    type = ElementAverageValue
    variable = iter
    outputs = 'console csv'
  [../]
  [./av_linesearch]
    type = ElementAverageValue
    variable = linesearch
    outputs = 'console csv'
  [../]
  [./av_ld]
    type = ElementAverageValue
    variable = ld
    outputs = 'console csv'
  [../]
  [./av_constr_added]
    type = ElementAverageValue
    variable = constr_added
    outputs = 'console csv'
  [../]
[]

[UserObjects]
  [./yield_strength]
    type = TensorMechanicsHardeningCubic
    value_0 = 20E6
    value_residual = 10E6
    internal_limit = 1
  [../]
  [./j2]
    type = TensorMechanicsPlasticJ2
    yield_strength = yield_strength
    yield_function_tolerance = 1.0E2
    internal_constraint_tolerance = 1.0E-7
    use_custom_returnMap = false
  [../]


  [./compressive_strength]
    type = TensorMechanicsHardeningCubic
    value_0 = 15E6
    value_residual = 5E6
    internal_limit = 1
  [../]
  [./cap]
    type = TensorMechanicsPlasticMeanCap
    a = -1
    strength = compressive_strength
    yield_function_tolerance = 1.0E2
    internal_constraint_tolerance = 1.0E-7
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '1.2E9 1.2E9'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./multi]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-5

    plastic_models = 'j2 cap'
    max_NR_iterations = 10
    deactivation_scheme = 'safe'
    min_stepsize = 1
    max_stepsize_for_dumb = 1
    tangent_operator = elastic # tangent operator is unimportant in this test

    debug_fspb = crash
    debug_jac_at_stress = '10E6 0 0 0 10E6 0 0 0 10E6'
    debug_jac_at_pm = '1E-2 1E-2'
    debug_jac_at_intnl = '0.05 0.05'
    debug_stress_change = 1E1
    debug_pm_change = '1E-6 1E-6'
    debug_intnl_change = '1E-6 1E-6'
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = paper5
  exodus = false
  csv = true
  perf_graph = true
[]

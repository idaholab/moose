# Plasticity models:
# WeakPlaneTensile with strength = 1000Pa
# WeakPlaneShear with cohesion = 0.1MPa and friction angle = 25
#
# Lame lambda = 1GPa.  Lame mu = 1.3GPa
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

[GlobalParams]
  volumetric_locking_correction=true
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
  [./av_iter]
    type = ElementAverageValue
    variable = iter
    outputs = 'console csv'
  [../]
[]

[UserObjects]
  [./wpt_str]
    type = TensorMechanicsHardeningConstant
    value = 1000
  [../]
  [./wpt]
    type = TensorMechanicsPlasticWeakPlaneTensile
    tensile_strength = wpt_str
    yield_function_tolerance = 1.0
    internal_constraint_tolerance = 1.0E-7
  [../]

  [./wps_c]
    type = TensorMechanicsHardeningConstant
    value = 1.0E5
  [../]
  [./wps_tan_phi]
    type = TensorMechanicsHardeningConstant
    value = 0.466
  [../]
  [./wps_tan_psi]
    type = TensorMechanicsHardeningConstant
    value = 0.087
  [../]
  [./wps]
    type = TensorMechanicsPlasticWeakPlaneShear
    cohesion = wps_c
    tan_friction_angle = wps_tan_phi
    tan_dilation_angle = wps_tan_psi
    smoother = 0
    yield_function_tolerance = 1.0
    internal_constraint_tolerance = 1.0E-7
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '1.0E9 1.3E9'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./multi]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-7

    plastic_models = 'wpt wps'
    max_NR_iterations = 5
    specialIC = 'joint'
    deactivation_scheme = 'safe'
    min_stepsize = 1
    max_stepsize_for_dumb = 1

    debug_fspb = crash
    debug_jac_at_stress = '10 0 0 0 10 0 0 0 10'
    debug_jac_at_pm = '1 1 1 1'
    debug_jac_at_intnl = '1 1 1 1'
    debug_stress_change = 1E1
    debug_pm_change = '1E-6 1E-6 1E-6 1E-6'
    debug_intnl_change = '1E-6 1E-6 1E-6 1E-6'
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = special_joint1
  exodus = false
  csv = true
[]

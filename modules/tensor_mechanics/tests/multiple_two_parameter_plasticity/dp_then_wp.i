# Use ComputeMultipleInelasticStress with two inelastic models: CappedDruckerPrager and CappedWeakPlane.
# The relative_tolerance and absolute_tolerance parameters are set very large so that
# only one iteration is performed.  This is the algorithm that FLAC uses to model
# jointed rocks, only Capped-Mohr-Coulomb is used instead of CappedDruckerPrager
#
# initial_stress = diag(1E3, 1E3, 1E3)
# The CappedDruckerPrager has tensile strength 3E2 and large cohesion,
# so the stress initially returns to diag(1E2, 1E2, 1E2)
# The CappedWeakPlane has tensile strength zero and large cohesion,
# so the stress returns to diag(1E2 - v/(1-v)*1E2, 1E2 - v/(1-v)*1E2, 0)
# where v=0.2 is the Poisson's ratio

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
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[BCs]
  [./x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'front back'
    function = 0
  [../]
  [./y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'front back'
    function = 0
  [../]
  [./z]
    type = FunctionPresetBC
    variable = disp_z
    boundary = 'front back'
    function = 0
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
  [./yield_fcn_dp]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn_wp]
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
  [./yield_fcn_dp_auxk]
    type = MaterialStdVectorAux
    index = 1    # this is the tensile yield function - it should be zero
    property = cdp_plastic_yield_function
    variable = yield_fcn_dp
  [../]
  [./yield_fcn_wp_auxk]
    type = MaterialStdVectorAux
    index = 1    # this is the tensile yield function - it should be zero
    property = cwp_plastic_yield_function
    variable = yield_fcn_wp
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
  [./f_dp]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn_dp
  [../]
  [./f_wp]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn_wp
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 300
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 1E4
  [../]
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 1E4
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 20
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 0
  [../]
  [./dp]
    type = TensorMechanicsPlasticDruckerPrager
    mc_cohesion = mc_coh
    mc_friction_angle = mc_phi
    mc_dilation_angle = mc_psi
    internal_constraint_tolerance = 1 # irrelevant here
    yield_function_tolerance = 1      # irrelevant here
  [../]
  [./wp_coh]
    type = TensorMechanicsHardeningConstant
    value = 1E4
  [../]
  [./wp_tanphi]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
  [./wp_tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.1111077
  [../]
  [./wp_t_strength]
    type = TensorMechanicsHardeningConstant
    value = 0
  [../]
  [./wp_c_strength]
    type = TensorMechanicsHardeningConstant
    value = 1E4
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.2
    youngs_modulus = 1E7
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    relative_tolerance = 1E4
    absolute_tolerance = 2
    inelastic_models = 'cdp cwp'
    perform_finite_strain_rotations = false
    initial_stress = '1E3 0 0  0 1E3 0  0 0 1E3'
  [../]
  [./cdp]
    type = CappedDruckerPragerStressUpdate
    name_prepender = cdp
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-5
    tip_smoother = 1E3
    smoothing_tol = 1E3
  [../]
  [./cwp]
    type = CappedWeakPlaneStressUpdate
    name_prepender = cwp
    cohesion = wp_coh
    tan_friction_angle = wp_tanphi
    tan_dilation_angle = wp_tanpsi
    tensile_strength = wp_t_strength
    compressive_strength = wp_c_strength
    tip_smoother = 1E3
    smoothing_tol = 1E3
    yield_function_tol = 1E-5
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = dp_then_wp
  csv = true
[]

# Use ComputeMultipleInelasticStress with two inelastic models: CappedDruckerPrager and CappedWeakPlane.
# The relative_tolerance and absolute_tolerance parameters are set very large so that
# only one iteration is performed.  This is the algorithm that FLAC uses to model
# jointed rocks, only Capped-Mohr-Coulomb is used instead of CappedDruckerPrager
#
# In this test "cycle_models=true" so that in the first timestep only
# CappedDruckerPrager is used, while in the second timestep only
# CappedWeakPlane is used.
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


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    incremental = true
    eigenstrain_names = ini_stress
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz'
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = 0
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = 0
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = 0
  [../]
[]

[AuxVariables]
  [./yield_fcn_dp]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn_wp]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tensile_cdp]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tensile_cwp]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
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
  [./tensile_cdp]
    type = MaterialStdVectorAux
    index = 1
    property = cdp_plastic_internal_parameter
    variable = tensile_cdp
  [../]
  [./tensile_cwp]
    type = MaterialStdVectorAux
    index = 1
    property = cwp_plastic_internal_parameter
    variable = tensile_cwp
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
  [./i_cdp]
    type = PointValue
    point = '0 0 0'
    variable = tensile_cdp
  [../]
  [./i_cwp]
    type = PointValue
    point = '0 0 0'
    variable = tensile_cwp
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
    youngs_modulus = 1.0
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '1E3 0 0  0 1E3 0  0 0 1E3'
    eigenstrain_name = ini_stress
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    relative_tolerance = 1E4
    absolute_tolerance = 2
    inelastic_models = 'cdp cwp'
    perform_finite_strain_rotations = false
    cycle_models = true
  [../]
  [./cdp]
    type = CappedDruckerPragerStressUpdate
    base_name = cdp
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-5
    tip_smoother = 1E3
    smoothing_tol = 1E3
  [../]
  [./cwp]
    type = CappedWeakPlaneStressUpdate
    base_name = cwp
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
  end_time = 2
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = cycled_dp_then_wp
  csv = true
[]

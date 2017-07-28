# Using CappedMohrCoulomb with compressive failure only
# checking for small deformation, with cubic hardening
# A single element is repeatedly compressed in z direction
# compressive_strength is set to 0.9Pa, compressive_strength_residual = 0.5Pa, and limit value = 1E-5
# This allows the hardening of the compressive strength to be observed

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
    function = '0'
  [../]
  [./y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'front back'
    function = '0'
  [../]
  [./z]
    type = FunctionPresetBC
    variable = disp_z
    boundary = 'front back'
    function = '-0.5E-6*z*t'
  [../]
[]

[AuxVariables]
  [./stress_I]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_II]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_III]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn]
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
[]

[AuxKernels]
  [./stress_I]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = stress_I
    scalar_type = MaxPrincipal
  [../]
  [./stress_II]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = MidPrincipal
    variable = stress_II
  [../]
  [./stress_III]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    scalar_type = MinPrincipal
    variable = stress_III
  [../]
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 3
    variable = yield_fcn
  [../]
  [./iter_auxk]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./intnl_auxk]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 1
    variable = intnl
  [../]
[]

[Postprocessors]
  [./s_I]
    type = PointValue
    point = '0 0 0'
    variable = stress_I
  [../]
  [./s_II]
    type = PointValue
    point = '0 0 0'
    variable = stress_II
  [../]
  [./s_III]
    type = PointValue
    point = '0 0 0'
    variable = stress_III
  [../]
  [./f]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
  [../]
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
  [../]
  [./intnl]
    type = PointValue
    point = '0 0 0'
    variable = intnl
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1E6
  [../]
  [./cs]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.9
    value_residual = 0.5
    internal_0 = -1E-5
    internal_limit = 0
  [../]
  [./coh]
    type = TensorMechanicsHardeningConstant
    value = 1E6
  [../]
  [./ang]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 2.0E6'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./compressive]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = ang
    dilation_angle = ang
    smoothing_tol = 0.0
    yield_function_tol = 1.0E-12
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = compressive
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 10
  dt = 1.0
  type = Transient
[]


[Outputs]
  file_base = small_deform_hard13
  csv = true
[]

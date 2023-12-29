# Mohr-Coulomb only
# apply equal stretches in x, y and z directions, to observe return to the MC tip
# Because of smoothing, the expected result is around
# Smax = Smid = Smin = 12.9
# The result is not exact because the smoothing is assymetrical.
# This test also employs a very small dilation angle, which makes return
# to the tip quite numerically difficult, so max_NR_iterations has been increased to 100

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
    generate_output = 'max_principal_stress mid_principal_stress min_principal_stress stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz'
  [../]
[]


[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '1E-6*x'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '1E-6*y'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '1E-6*z'
  [../]
[]

[AuxVariables]
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
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    index = 6
    property = plastic_yield_function
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
    index = 0
    variable = intnl
  [../]
[]

[Postprocessors]
  [./s_max]
    type = PointValue
    point = '0 0 0'
    variable = max_principal_stress
  [../]
  [./s_mid]
    type = PointValue
    point = '0 0 0'
    variable = mid_principal_stress
  [../]
  [./s_min]
    type = PointValue
    point = '0 0 0'
    variable = min_principal_stress
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
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 10
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 30
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 1E-4
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1E7
    poissons_ratio = 0.3
  [../]
  [./mc]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = ts
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    smoothing_tol = 5
    yield_function_tol = 1.0E-9
    max_NR_iterations = 100
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = mc
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform25
  csv = true
[]

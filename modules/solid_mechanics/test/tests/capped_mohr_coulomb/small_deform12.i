# Using CappedMohrCoulomb with compressive failure only
# checking for small deformation
# A single element is stretched equally in all directions.
# This causes the return direction to be along the sigma_I = sigma_II = sigma_III line
# compressive_strength is set to 1Pa, and smoothing_tol = 0.1Pa
# The smoothed yield function comes from two smoothing operations.
# The first is on sigma_I and sigma_II (sigma_I >= sigma_II >= sigma_III):
# yf = -sigma_I + ismoother(0) - compressive_strength
#    = -sigma_I + (0.5 * smoothing_tol - smoothing_tol / Pi) - compressive_strength
#    = -sigma_I + 0.018169 - 1
# The second has the argument of ismoother equal to -0.018169.
# ismoother(-0.018169) = 0.5 * (-0.018169 + 0.1) - 0.1 * cos (0.5 * Pi * -0.018169 / 0.1) / Pi
#                     = 0.010372
# So the final yield function is
# yf = -sigma_I + 0.018169 + 0.010372 - 1 = -sigma_I + 0.028541 - 1
# However, because of the asymmetry in smoothing (the yield function is obtained
# by first smoothing -sigma_I-cs and -sigma_II-cs, and then by smoothing this
# result with -sigma_III-cs) the result is sigma_I > sigma_II = sigma_III


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
    function = '-1E-6*x'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '-1E-6*y'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '-1E-6*z'
  [../]
[]

[AuxVariables]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = yield_fcn
  [../]
[]

[Postprocessors]
  [./s_I]
    type = PointValue
    point = '0 0 0'
    variable = max_principal_stress
  [../]
  [./s_II]
    type = PointValue
    point = '0 0 0'
    variable = mid_principal_stress
  [../]
  [./s_III]
    type = PointValue
    point = '0 0 0'
    variable = min_principal_stress
  [../]
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
    variable = yield_fcn
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 1
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
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 2.0E6'
  [../]
  [./tensile]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = ang
    dilation_angle = ang
    smoothing_tol = 0.1
    yield_function_tol = 1.0E-12
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = tensile
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform12
  csv = true
[]

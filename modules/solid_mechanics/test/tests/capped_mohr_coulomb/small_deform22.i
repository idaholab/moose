# Mohr-Coulomb only
# apply stretches in x direction and smaller stretches in the y direction
# to observe return to the MC plane
# This tests uses hardening of the cohesion.  The returned configuration
# should obey
# 0 = 0.5 * (Smax - Smin) + 0.5 * (Smax + Smin) * sin(phi) - C cos(phi)
# which allows inference of C.

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
    function = '0.4E-6*x*t'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '0.1E-6*y*t'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '0'
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
    type = TensorMechanicsHardeningCubic
    value_0 = 10
    value_residual = 20
    internal_limit = 5E-6
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 30
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 30
    convert_to_radians = true
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
    smoothing_tol = 0
    yield_function_tol = 1.0E-9
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = mc
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 10
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform_hard21
  csv = true
[]

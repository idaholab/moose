# apply repeated stretches in z directions, and smaller stretches along the x and y directions,
# so that sigma_mid = sigma_min (approximately),
# which means that lode angle = -30deg.
# The allows yield surface in meridional plane to be mapped out


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
    function = '0.25E-6*x*sin(t)'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '0.25E-6*y*sin(t)'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '1E-6*z*t'
  [../]
[]

[AuxVariables]
  [./mc_int]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./mc_int_auxk]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_internal_parameter
    variable = mc_int
  [../]
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    index = 6
    property = plastic_yield_function
    variable = yield_fcn
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
  [./internal]
    type = PointValue
    point = '0 0 0'
    variable = mc_int
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
    value = 0.5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E7'
  [../]
  [./mc]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = ts
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    smoothing_tol = 5.0
    yield_function_tol = 1.0E-7
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = mc
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 30
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform24
  csv = true
[]

# Mohr-Coulomb only
# apply repeated stretches in x, y and z directions, so that mean_stress = 0
# This maps out the yield surface in the octahedral plane for zero mean stress

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
    function = '2E-6*x*sin(t)'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '2E-6*y*sin(2*t)'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '-2E-6*z*(sin(t)+sin(2*t))'
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
  [./f]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
  [../]
  [./f0]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
  [../]
  [./f1]
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
    value = 20
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 1E-12
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
    smoothing_tol = 1
    yield_function_tol = 1.0E-9
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = mc
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 100
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform21
  csv = true
[]

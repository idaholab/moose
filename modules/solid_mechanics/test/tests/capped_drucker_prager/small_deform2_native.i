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
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz'
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '-1.5E-6*x+2E-6*x*sin(t)'
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
    index = 0
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
    value = 1000
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 1000
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
    value = 0
  [../]
  [./dp]
    type = TensorMechanicsPlasticDruckerPrager
    mc_cohesion = mc_coh
    mc_friction_angle = mc_phi
    mc_dilation_angle = mc_psi
    mc_interpolation_scheme = native
    internal_constraint_tolerance = 1 # irrelevant here
    yield_function_tolerance = 1      # irrelevant here
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E7'
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = cdp
    perform_finite_strain_rotations = false
  [../]
  [./cdp]
    type = CappedDruckerPragerStressUpdate
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-8
    tip_smoother = 4
    smoothing_tol = 1E-5
  [../]
[]


[Executioner]
  end_time = 100
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform2_native
  exodus = false
  [./csv]
    type = CSV
  [../]
[]

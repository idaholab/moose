# Using CappedMohrCoulomb with tensile failure only
# A single element is incrementally stretched in the in the z and x directions
# This causes the return direction to be along the hypersurface sigma_I = sigma_II
# and the resulting stresses are checked to lie on the expected yield surface

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
    strain = finite
    incremental = true
    generate_output = 'max_principal_stress mid_principal_stress min_principal_stress stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz'
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '4*x*t'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '1*y*t'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '4*z*t'
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
    value = 1E6
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
    C_ijkl = '0 2.0'
  [../]
  [./tensile]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = ang
    dilation_angle = ang
    smoothing_tol = 0.5
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
  dt = 0.1
  type = Transient
[]


[Outputs]
  file_base = small_deform7
  csv = true
[]

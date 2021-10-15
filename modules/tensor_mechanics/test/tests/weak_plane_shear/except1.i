# checking for exception error messages
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
  generate_output = 'stress_xz stress_zx stress_yz stress_zz'
[]

[BCs]
  [bottomx]
    type = DirichletBC
    variable = x_disp
    boundary = back
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    variable = y_disp
    boundary = back
    value = 0.0
  []
  [bottomz]
    type = DirichletBC
    variable = z_disp
    boundary = back
    value = 0.0
  []

  [topx]
    type = DirichletBC
    variable = x_disp
    boundary = front
    value = 8E-6
  []
  [topy]
    type = DirichletBC
    variable = y_disp
    boundary = front
    value = 6E-6
  []
  [topz]
    type = DirichletBC
    variable = z_disp
    boundary = front
    value = 1E-6
  []
[]

[AuxVariables]
  [yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = yield_fcn
  []
[]

[Postprocessors]
  [s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  []
  [s_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  []
  [s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  []
  [f]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
  []
[]

[UserObjects]
  [coh]
    type = TensorMechanicsHardeningConstant
    value = 1
  []
  [tanphi]
    type = TensorMechanicsHardeningConstant
    value = 0.5
  []
  [tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.55
  []
  [wps]
    type = TensorMechanicsPlasticWeakPlaneShear
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    smoother = 0
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-3
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E6'
  []
  [mc]
    type = ComputeMultiPlasticityStress
    plastic_models = wps
    transverse_direction = '0 0 1'
    ep_plastic_tolerance = 1E-3
  []
[]

[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]

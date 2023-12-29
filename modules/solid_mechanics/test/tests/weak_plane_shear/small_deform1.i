# deformations are applied so that the trial stress is
# shear = 10, normalstress = 2
#
# Cohesion is chosen to be 1, and friction angle = 26.565, so tan(friction_angle) = 1/2
# This means that (shear, normalstress) = (0, 2) is the apex
# of the shear envelope
#
# Poisson's ratio is chosen to be zero, and Lame mu = 1E6,
# so the return must solve
# f = 0
# shear = shear_trial - (1/2)*mu*ga = 10 - 0.5E6*ga
# normalstress = normalstress - mu*tan(dilation)*ga
#
# Finally, tan(dilation) = 2/18 is chosen.
#
# Then the returned value should have
# shear = 1, normalstress = 0
#
# Here shear = sqrt(s_yz^2 + s_xz^2)
[GlobalParams]
  displacements = 'x_disp y_disp z_disp'
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
    value = 0.1111077
  []
  [wps]
    type = TensorMechanicsPlasticWeakPlaneShear
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    smoother = 0
    yield_function_tolerance = 1E-6
    internal_constraint_tolerance = 1E-6
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
    ep_plastic_tolerance = 1E-5
    debug_fspb = crash
  []
[]

[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]

[Outputs]
  csv = true
[]

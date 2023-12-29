# Checking solution of hardening
# A single element is stretched by 1E-6 in z direction.
#
# Young's modulus = 20 MPa.  Tensile strength = 10 Exp(-1E6*q) Pa
#
# The trial stress is
# trial_stress_zz = Youngs Modulus*Strain = 2E7*1E-6 = 20 Pa
#
# Therefore the equations we have to solve are
# 0 = f = stress_zz - 10 Exp(-1E6*q)
# 0 = epp = ga - (20 - stress_zz)/2E7
# 0 = intnl = q - ga
#
# The result is
# q = 0.76803905E-6
# stress_zz = 4.6392191 Pa
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
  generate_output = 'stress_zz'
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
    value = 0
  []
  [topy]
    type = DirichletBC
    variable = y_disp
    boundary = front
    value = 0
  []
  [topz]
    type = FunctionDirichletBC
    variable = z_disp
    boundary = front
    function = 1E-6*t
  []
[]

[AuxVariables]
  [wpt_internal]
    order = CONSTANT
    family = MONOMIAL
  []
  [yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [wpt_internal]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = wpt_internal
  []
  [yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = yield_fcn
  []
[]

[Postprocessors]
  [wpt_internal]
    type = PointValue
    point = '0 0 0'
    variable = wpt_internal
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
  [str]
    type = TensorMechanicsHardeningExponential
    value_0 = 10
    value_residual = 0
    rate = 1E6
  []
  [wpt]
    type = TensorMechanicsPlasticWeakPlaneTensile
    tensile_strength = str
    yield_function_tolerance = 1E-6
    internal_constraint_tolerance = 1E-11
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E7'
  []
  [mc]
    type = ComputeMultiPlasticityStress
    plastic_models = wpt
    transverse_direction = '0 0 1'
    ep_plastic_tolerance = 1E-11
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

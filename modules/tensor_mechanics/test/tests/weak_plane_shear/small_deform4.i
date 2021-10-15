# apply a pure tension, then some shear
# the BCs are designed to map out the yield function, showing
# the affect of 'cap' smoothing
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
    type = FunctionDirichletBC
    variable = x_disp
    boundary = front
    function = 'if(t<1E-6,0,3*(t-1E-6)*(t-1E-6)*1E6)'
  []
  [topy]
    type = FunctionDirichletBC
    variable = y_disp
    boundary = front
    function = 'if(t<1E-6,0,5*(t-1E-6)*(t-1E-6)*1E6)'
  []
  [topz]
    type = FunctionDirichletBC
    variable = z_disp
    boundary = front
    function = 'if(t<1E-6,t,1E-6)'
  []
[]

[AuxVariables]
  [yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  []
  [iter]
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
  [iter_auxk]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
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
  [iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
  []
[]

[UserObjects]
  [coh]
    type = TensorMechanicsHardeningConstant
    value = 1E3
  []
  [tanphi]
    type = TensorMechanicsHardeningConstant
    value = 1
  []
  [tanpsi]
    type = TensorMechanicsHardeningConstant
    value = 0.08748866
  []
  [wps]
    type = TensorMechanicsPlasticWeakPlaneShear
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tip_scheme = cap
    smoother = 0
    cap_rate = 0.001
    cap_start = -1000.0
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-6
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '1E9 0.5E9'
  []
  [mc]
    type = ComputeMultiPlasticityStress
    ep_plastic_tolerance = 1E-4
    plastic_models = wps
    transverse_direction = '0 0 1'
    debug_fspb = crash
    debug_jac_at_stress = '1E4 2E4 3E4 2E4 -4E4 5E4 3E4 5E4 6E8'
    debug_jac_at_pm = 1
    debug_jac_at_intnl = 1
    debug_stress_change = 1E-3
    debug_pm_change = 1E-5
    debug_intnl_change = 1E-5
  []
[]

[Executioner]
  end_time = 2E-6
  dt = 1E-7
  type = Transient
[]

[Outputs]
  csv = true
[]

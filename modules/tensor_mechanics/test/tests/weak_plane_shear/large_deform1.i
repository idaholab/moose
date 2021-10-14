# rotate the mesh by 90degrees
# then pull in the z direction - should be no plasticity
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
  decomposition_method = EigenSolution
  generate_output = 'stress_xz stress_zx stress_yz stress_zz'
[]

[BCs]
  # rotate:
  # ynew = c*y + s*z.  znew = -s*y + c*z
  [bottomx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = back
    function = '0'
  []
  [bottomy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = back
    function = '0*y+1*z-y'
  []
  [bottomz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = back
    function = '-1*y+0*z-z+if(t>0,0.5-y,0)' # note that this uses original nodal values of (x,y,z)
  []

  [topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = front
    function = '0'
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = front
    function = '0*y+1*z-y'
  []
  [topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '-1*y+0*z-z+if(t>0,0.5-y,0)' # note that this uses original nodal values of (x,y,z)
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
    smoother = 0.5
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
  start_time = -1
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  exodus = true
[]

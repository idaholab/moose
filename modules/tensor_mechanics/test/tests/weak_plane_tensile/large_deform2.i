# large strain with weak-plane normal rotating with mesh
# First rotate mesh 45deg about x axis
# Then apply stretch in the y=z direction.
# This should create a pure tensile load (no shear), which
# should return to the yield surface.
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
  generate_output = 'stress_xx stress_yy stress_yz stress_zz'
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
    function = '0.70710678*y+0.70710678*z-y'
  []
  [bottomz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = back
    function = '-0.70710678*y+0.70710678*z-z'
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
    function = '0.70710678*y+0.70710678*z-y+if(t>0,1,0)'
  []
  [topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '-0.70710678*y+0.70710678*z-z+if(t>0,1,0)'
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
  [s_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  []
  [s_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
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
  [str]
    type = TensorMechanicsHardeningConstant
    value = 1.0E6
  []
  [wpt]
    type = TensorMechanicsPlasticWeakPlaneTensile
    tensile_strength = str
    yield_function_tolerance = 1E-7
    internal_constraint_tolerance = 1E-5
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
    ep_plastic_tolerance = 1E-9
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

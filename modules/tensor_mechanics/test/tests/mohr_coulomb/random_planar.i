# apply many random large deformations, checking that the algorithm returns correctly to
# the yield surface each time.


[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  ny = 1250
  nz = 1
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 1250
  zmin = 0
  zmax = 1
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]


[ICs]
  [./x]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_x
  [../]
  [./y]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_y
  [../]
  [./z]
    type = RandomIC
    min = -0.1
    max = 0.1
    variable = disp_z
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '0'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '0'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '0'
  [../]
[]

[AuxVariables]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
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
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
[]

[Postprocessors]
  [./yield_fcn_at_zero]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
    outputs = 'console'
  [../]
  [./should_be_zero]
    type = FunctionValuePostprocessor
    function = should_be_zero_fcn
  [../]
  [./av_iter]
    type = ElementAverageValue
    variable = iter
    outputs = 'console'
  [../]
[]

[Functions]
  [./should_be_zero_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-3,0,a)'
    symbol_names = 'a'
    symbol_values = 'yield_fcn_at_zero'
  [../]
[]

[UserObjects]
  [./coh]
    type = TensorMechanicsHardeningCubic
    value_0 = 1000
    value_residual = 100
    internal_limit = 4
  [../]
  [./phi]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.8
    value_residual = 0.3
    internal_limit = 2
  [../]
  [./psi]
    type = TensorMechanicsHardeningConstant
    value = 15
    convert_to_radians = true
  [../]
  [./mc]
    type = TensorMechanicsPlasticMohrCoulombMulti
    cohesion = coh
    friction_angle = phi
    dilation_angle = psi
    yield_function_tolerance = 1E-3
    shift = 1E-10
    internal_constraint_tolerance = 1E-6
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0.7E7 1E7'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-10
    plastic_models = mc
    min_stepsize = 1
    max_stepsize_for_dumb = 1
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = random_planar
  exodus = false
  [./csv]
    type = CSV
    [../]
[]

# Using CappedMohrCoulomb with Mohr-Coulomb failure only
# Plasticity models:
# Cohesion = 1MPa
# Friction angle = dilation angle = 0.5
#
# Lame lambda = 1GPa.  Lame mu = 1.3GPa
#
# A line of elements is perturbed randomly, and return to the yield surface at each quadpoint is checked

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1000
  ny = 1234
  nz = 1
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 1234
  zmin = 0
  zmax = 1
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
  [./f0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f3]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f4]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f5]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./f0]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 6
    variable = f0
  [../]
  [./f1]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 7
    variable = f1
  [../]
  [./f2]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 8
    variable = f2
  [../]
  [./f3]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 9
    variable = f3
  [../]
  [./f4]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 10
    variable = f4
  [../]
  [./f5]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 11
    variable = f5
  [../]
  [./int0]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = int0
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
[]

[Postprocessors]
  [./tot_iters]
    type = ElementIntegralMaterialProperty
    mat_prop = plastic_NR_iterations
    outputs = console
  [../]
  [./intnl_max]
    type = ElementExtremeValue
    variable = int0
    outputs = console
  [../]
  [./raw_f0]
    type = ElementExtremeValue
    variable = f0
    outputs = console
  [../]
  [./raw_f1]
    type = ElementExtremeValue
    variable = f1
    outputs = console
  [../]
  [./raw_f2]
    type = ElementExtremeValue
    variable = f2
    outputs = console
  [../]
  [./raw_f3]
    type = ElementExtremeValue
    variable = f3
    outputs = console
  [../]
  [./raw_f4]
    type = ElementExtremeValue
    variable = f4
    outputs = console
  [../]
  [./raw_f5]
    type = ElementExtremeValue
    variable = f5
    outputs = console
  [../]
  [./iter]
    type = ElementExtremeValue
    variable = iter
    outputs = console
  [../]
  [./f0]
    type = FunctionValuePostprocessor
    function = should_be_zero0_fcn
  [../]
  [./f1]
    type = FunctionValuePostprocessor
    function = should_be_zero1_fcn
  [../]
  [./f2]
    type = FunctionValuePostprocessor
    function = should_be_zero2_fcn
  [../]
  [./f3]
    type = FunctionValuePostprocessor
    function = should_be_zero3_fcn
  [../]
  [./f4]
    type = FunctionValuePostprocessor
    function = should_be_zero4_fcn
  [../]
  [./f5]
    type = FunctionValuePostprocessor
    function = should_be_zero5_fcn
  [../]
[]

[Functions]
  [./should_be_zero0_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-1,0,a)'
    symbol_names = 'a'
    symbol_values = 'raw_f0'
  [../]
  [./should_be_zero1_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-1,0,a)'
    symbol_names = 'a'
    symbol_values = 'raw_f1'
  [../]
  [./should_be_zero2_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-1,0,a)'
    symbol_names = 'a'
    symbol_values = 'raw_f2'
  [../]
  [./should_be_zero3_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-1,0,a)'
    symbol_names = 'a'
    symbol_values = 'raw_f3'
  [../]
  [./should_be_zero4_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-1,0,a)'
    symbol_names = 'a'
    symbol_values = 'raw_f4'
  [../]
  [./should_be_zero5_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-1,0,a)'
    symbol_names = 'a'
    symbol_values = 'raw_f5'
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1E12
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 1E12
  [../]
  [./coh]
    type = TensorMechanicsHardeningCubic
    value_0 = 1E6
    value_residual = 0
    internal_limit = 1
  [../]
  [./ang]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.9
    value_residual = 0.2
    internal_limit = 1
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '1E9 1.3E9'
  [../]
  [./tensile]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = ang
    dilation_angle = ang
    smoothing_tol = 1E5
    max_NR_iterations = 100
    yield_function_tol = 1.0E-1
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = tensile
    perform_finite_strain_rotations = false
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = random3
  csv = true
[]

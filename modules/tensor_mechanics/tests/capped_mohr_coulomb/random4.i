# Using CappedMohrCoulomb
# Plasticity models:
# Tensile strength = 0.1MPa
# Compressive strength = 1.0MPa
# Cohesion = 1MPa
# Friction angle = dilation angle = 0.5
#
# Lame lambda = 1GPa.  Lame mu = 1.3GPa
#
# A line of elements is perturbed randomly, and return to the yield surface at each quadpoint is checked

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  ny = 1234
  nz = 1
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 1234
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
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'front back'
    function = '0'
  [../]
  [./y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'front back'
    function = '0'
  [../]
  [./z]
    type = FunctionPresetBC
    variable = disp_z
    boundary = 'front back'
    function = '0'
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
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
  [./f6]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f7]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f8]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f9]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f10]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./f11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./int1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./f0]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f0
  [../]
  [./f1]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f1
  [../]
  [./f2]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 2
    variable = f2
  [../]
  [./f3]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 3
    variable = f3
  [../]
  [./f4]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 4
    variable = f4
  [../]
  [./f5]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 5
    variable = f5
  [../]
  [./f6]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 6
    variable = f6
  [../]
  [./f7]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 7
    variable = f7
  [../]
  [./f8]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 8
    variable = f8
  [../]
  [./f9]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 9
    variable = f9
  [../]
  [./f10]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 10
    variable = f10
  [../]
  [./f11]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 11
    variable = f11
  [../]
  [./int0]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = int0
  [../]
  [./int1]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 1
    variable = int1
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
  [./intnl0_max]
    type = ElementExtremeValue
    variable = int0
    outputs = console
  [../]
  [./intnl1_max]
    type = ElementExtremeValue
    variable = int1
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
  [./raw_f6]
    type = ElementExtremeValue
    variable = f6
    outputs = console
  [../]
  [./raw_f7]
    type = ElementExtremeValue
    variable = f7
    outputs = console
  [../]
  [./raw_f8]
    type = ElementExtremeValue
    variable = f8
    outputs = console
  [../]
  [./raw_f9]
    type = ElementExtremeValue
    variable = f9
    outputs = console
  [../]
  [./raw_f10]
    type = ElementExtremeValue
    variable = f10
    outputs = console
  [../]
  [./raw_f11]
    type = ElementExtremeValue
    variable = f11
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
  [./f6]
    type = FunctionValuePostprocessor
    function = should_be_zero6_fcn
  [../]
  [./f7]
    type = FunctionValuePostprocessor
    function = should_be_zero7_fcn
  [../]
  [./f8]
    type = FunctionValuePostprocessor
    function = should_be_zero8_fcn
  [../]
  [./f9]
    type = FunctionValuePostprocessor
    function = should_be_zero9_fcn
  [../]
  [./f10]
    type = FunctionValuePostprocessor
    function = should_be_zero10_fcn
  [../]
  [./f11]
    type = FunctionValuePostprocessor
    function = should_be_zero11_fcn
  [../]
[]

[Functions]
  [./should_be_zero0_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f0'
  [../]
  [./should_be_zero1_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f1'
  [../]
  [./should_be_zero2_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f2'
  [../]
  [./should_be_zero3_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f3'
  [../]
  [./should_be_zero4_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f4'
  [../]
  [./should_be_zero5_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f5'
  [../]
  [./should_be_zero6_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f6'
  [../]
  [./should_be_zero7_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f7'
  [../]
  [./should_be_zero8_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f8'
  [../]
  [./should_be_zero9_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f9'
  [../]
  [./should_be_zero10_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f10'
  [../]
  [./should_be_zero11_fcn]
    type = ParsedFunction
    value = 'if(a<1E-1,0,a)'
    vars = 'a'
    vals = 'raw_f11'
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningCubic
    value_0 = 1E5
    value_residual = 2E5
    internal_limit = 1
  [../]
  [./cs]
    type = TensorMechanicsHardeningCubic
    value_0 = 1E6
    value_residual = 0.5E6
    internal_limit = 1
  [../]
  [./coh]
    type = TensorMechanicsHardeningCubic
    value_0 = 1E6
    value_residual = 0
    internal_limit = 1
  [../]
  [./phi]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.9
    value_residual = 0.2
    internal_limit = 1
  [../]
  [./psi]
    type = TensorMechanicsHardeningCubic
    value_0 = 0.5
    value_residual = 0.1
    internal_limit = 1
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '1E9 1.3E9'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./tensile]
    type = CappedMohrCoulombStressUpdate
    tensile_strength = ts
    compressive_strength = cs
    cohesion = coh
    friction_angle = phi
    dilation_angle = psi
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
  file_base = random4
  csv = true
[]

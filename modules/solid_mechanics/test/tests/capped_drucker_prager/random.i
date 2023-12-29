# capped drucker-prager
# apply many random large deformations, checking that the algorithm returns correctly to
# the yield surface each time.


[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1000
  ny = 125
  nz = 1
  xmin = 0
  xmax = 1000
  ymin = 0
  ymax = 125
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
    strain = finite
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
  [./shear_yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./tensile_yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./compressive_yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./shear_yield_fcn_auxk]
    type = MaterialStdVectorAux
    index = 0
    property = plastic_yield_function
    variable = shear_yield_fcn
  [../]
  [./tensile_fcn_auxk]
    type = MaterialStdVectorAux
    index = 1
    property = plastic_yield_function
    variable = tensile_yield_fcn
  [../]
  [./compressive_yield_fcn_auxk]
    type = MaterialStdVectorAux
    index = 2
    property = plastic_yield_function
    variable = compressive_yield_fcn
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
[]

[Postprocessors]
  [./shear_max]
    type = ElementExtremeValue
    variable = shear_yield_fcn
    outputs = 'console'
  [../]
  [./tensile_max]
    type = ElementExtremeValue
    variable = tensile_yield_fcn
    outputs = 'console'
  [../]
  [./compressive_max]
    type = ElementExtremeValue
    variable = compressive_yield_fcn
    outputs = 'console'
  [../]
  [./should_be_zero_shear]
    type = FunctionValuePostprocessor
    function = shear_should_be_zero_fcn
  [../]
  [./should_be_zero_compressive]
    type = FunctionValuePostprocessor
    function = compressive_should_be_zero_fcn
  [../]
  [./should_be_zero_tensile]
    type = FunctionValuePostprocessor
    function = tensile_should_be_zero_fcn
  [../]
  [./av_iter]
    type = ElementAverageValue
    variable = iter
    outputs = 'console'
  [../]
[]

[Functions]
  [./shear_should_be_zero_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-3,0,a)'
    symbol_names = 'a'
    symbol_values = 'shear_max'
  [../]
  [./tensile_should_be_zero_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-3,0,a)'
    symbol_names = 'a'
    symbol_values = 'tensile_max'
  [../]
  [./compressive_should_be_zero_fcn]
    type = ParsedFunction
    expression = 'if(a<1E-3,0,a)'
    symbol_names = 'a'
    symbol_values = 'compressive_max'
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1000
  [../]
  [./cs]
    type = TensorMechanicsHardeningConstant
    value = 1000
  [../]
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 1E3
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 30
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 5
    convert_to_radians = true
  [../]
  [./dp]
    type = TensorMechanicsPlasticDruckerPrager
    mc_cohesion = mc_coh
    mc_friction_angle = mc_phi
    mc_dilation_angle = mc_psi
    yield_function_tolerance = 1      # irrelevant here
    internal_constraint_tolerance = 1 # irrelevant here
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0.7E7 1E7'
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = dp
    perform_finite_strain_rotations = false
  [../]
  [./dp]
    type = CappedDruckerPragerStressUpdate
    DP_model = dp
    tensile_strength = ts
    compressive_strength = cs
    yield_function_tol = 1E-3
    tip_smoother = 0.1E3
    smoothing_tol = 0.1E3
    max_NR_iterations = 1000
    small_dilation = false
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = random
  exodus = false
  [./csv]
    type = CSV
  [../]
[]

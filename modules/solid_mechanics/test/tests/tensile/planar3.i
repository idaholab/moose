# checking for small deformation
# A single element is stretched by 1E-6m in the z and x directions, with lame mu = 1E6, so trial stress is 2Pa in those directions
# tensile_strength is set to 1Pa
# Then the final stress should return to the z and x stresses being 1.0 (up to tolerance), and internal parameter = (0.5+0.5)E-6 = 1.0E-6
# Using 'planar' Tensile plasticity

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
[]


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    incremental = true
    strain = finite
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_yz stress_zz'
  [../]
[]

[BCs]
  [./x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'front back'
    function = '1E-6*x'
  [../]
  [./y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'front back'
    function = '0E-6*y'
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'front back'
    function = '1E-6*z'
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
  [./iter]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./intnl]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./f0_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f0
  [../]
  [./f1_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f1
  [../]
  [./f2_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 2
    variable = f2
  [../]
  [./iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  [../]
  [./intnl_auxk]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = intnl
  [../]
[]

[Postprocessors]
  [./s_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./s_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./s_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./s_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./s_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./f0]
    type = PointValue
    point = '0 0 0'
    variable = f0
  [../]
  [./f1]
    type = PointValue
    point = '0 0 0'
    variable = f1
  [../]
  [./f2]
    type = PointValue
    point = '0 0 0'
    variable = f2
  [../]
  [./iter]
    type = PointValue
    point = '0 0 0'
    variable = iter
    outputs = console
  [../]
  [./intnl]
    type = PointValue
    point = '0 0 0'
    variable = intnl
  [../]
[]

[UserObjects]
  [./hard]
    type = TensorMechanicsHardeningConstant
    value = 1
  [../]
  [./tens]
    type = TensorMechanicsPlasticTensileMulti
    tensile_strength = hard
    yield_function_tolerance = 1E-6
    internal_constraint_tolerance = 1E-5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E6'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-5
    max_NR_iterations = 4
    min_stepsize = 1
    plastic_models = tens
    debug_fspb = crash
    debug_jac_at_stress = '1 2 3 2 -4 -5 3 -5 10'
    debug_jac_at_pm = '0.1 0.2 0.3'
    debug_jac_at_intnl = 1E-6
    debug_stress_change = 1E-6
    debug_pm_change = '1E-6 1E-6 1E-6'
    debug_intnl_change = 1E-6
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = planar3
  exodus = false
  [./csv]
    type = CSV
    [../]
[]

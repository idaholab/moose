# checking for small deformation
# A single element is stretched by 1E-6m in x,y and z directions.
# stress_zz = Youngs Modulus*Strain = 2E6*1E-6 = 2 Pa
# wpt_tensile_strength is set to 1Pa
# Then the final stress should return to the yeild surface and its value should be 1pa.
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
  generate_output = 'stress_xz stress_yz stress_zz'
[]

[BCs]
  [bottomx]
    type = DirichletBC
    variable = disp_x
    boundary = back
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    variable = disp_y
    boundary = back
    value = 0.0
  []
  [bottomz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []

  [topx]
    type = DirichletBC
    variable = disp_x
    boundary = front
    value = 0E-6
  []
  [topy]
    type = DirichletBC
    variable = disp_y
    boundary = front
    value = 0E-6
  []
  [topz]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 1E-6
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
  [str]
    type = TensorMechanicsHardeningConstant
    value = 1
  []
  [wpt]
    type = TensorMechanicsPlasticWeakPlaneTensileN
    tensile_strength = str
    yield_function_tolerance = 1E-6
    internal_constraint_tolerance = 1E-5
    normal_vector = '0 0 1'
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
    plastic_models = wpt
    ep_plastic_tolerance = 1E-5
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

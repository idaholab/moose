# checking for small deformation
# A single element is stretched by "ep" in the z and x directions
# This causes the return direction to be along the hypersurface sigma_I = sigma_II
# tensile_strength is set to 1Pa, tip_smoother = 0, edge_smoother = 25degrees
# Then A + B + C = 0.609965
#
# The trial stress is (la, 0, la), with mean stress 2la/3, and bar(sigma)=sqrt(secondInvariant)=la/sqrt(3)
# If this sits on the yield surface then
# 2la/3 + la*K/sqrt(3) - 1 = 0
# So la = 0.9815.  Therefore, with young's modulus = 2MPa, we need "ep" = 0.9815/4.  I set
# "ep" = 0.25 and observe a tiny amount of yielding


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
    function = '0.25E-6*x'
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
    function = '0.25E-6*z'
  [../]
[]

[AuxVariables]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = yield_fcn
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
  [./f]
    type = PointValue
    point = '0 0 0'
    variable = yield_fcn
  [../]
[]

[UserObjects]
  [./ts]
    type = TensorMechanicsHardeningConstant
    value = 1
  [../]
  [./mc]
    type = TensorMechanicsPlasticTensile
    tensile_strength = ts
    yield_function_tolerance = 1E-6
    tensile_tip_smoother = 0.0
    internal_constraint_tolerance = 1E-5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 2.0E6'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    ep_plastic_tolerance = 1E-5
    plastic_models = mc
    debug_fspb = crash
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform3
  exodus = false
  [./csv]
    type = CSV
    [../]
[]

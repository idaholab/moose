# Checking internal-parameter evolution
# A single element is stretched by 1E-6*t in z directions.
#
# Young's modulus = 20 MPa.  Tensile strength = 10 Pa
#
# There are two time steps.
# In the first
# trial stress_zz = Youngs Modulus*Strain = 2E7*1E-6 = 20 Pa
# so this returns to stress_zz = 10 Pa, and half of the deformation
# goes to plastic strain, yielding ep_zz_plastic = 0.5E-6
# In the second
# trial stress_zz = 10 + Youngs Modulus*(Strain increment) = 10 + 2E7*1E-6 = 30 Pa
# so this returns to stress_zz = 10 Pa, and all of the deformation
# goes to plastic strain, yielding ep_zz_plastic increment = 1E-6,
# so total plastic strain_zz = 1.5E-6.


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


[Variables]
  [./x_disp]
  [../]
  [./y_disp]
  [../]
  [./z_disp]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'x_disp y_disp z_disp'
  [../]
[]


[BCs]
  [./bottomx]
    type = PresetBC
    variable = x_disp
    boundary = back
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    variable = y_disp
    boundary = back
    value = 0.0
  [../]
  [./bottomz]
    type = PresetBC
    variable = z_disp
    boundary = back
    value = 0.0
  [../]

  [./topx]
    type = PresetBC
    variable = x_disp
    boundary = front
    value = 0
  [../]
  [./topy]
    type = PresetBC
    variable = y_disp
    boundary = front
    value = 0
  [../]
  [./topz]
    type = FunctionPresetBC
    variable = z_disp
    boundary = front
    function = 1E-6*t
  [../]
[]

[AuxVariables]
  [./wpt_internal]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./wpt_internal]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = wpt_internal
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./yield_fcn_auxk]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = yield_fcn
  [../]
[]

[Postprocessors]
  [./wpt_internal]
    type = PointValue
    point = '0 0 0'
    variable = wpt_internal
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
  [./str]
    type = TensorMechanicsHardeningConstant
    value = 10
  [../]
  [./wpt]
    type = TensorMechanicsPlasticWeakPlaneTensile
    tensile_strength = str
    yield_function_tolerance = 1E-6
    internal_constraint_tolerance = 1E-11
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '0 1E7'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'x_disp y_disp z_disp'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    plastic_models = wpt
    transverse_direction = '0 0 1'
    ep_plastic_tolerance = 1E-11
  [../]
[]


[Executioner]
  end_time = 2
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform_hard1
  exodus = false
  [./csv]
    type = CSV
    [../]
[]

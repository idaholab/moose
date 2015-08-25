# checking for small deformation
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
    displacements = 'disp_x disp_y disp_z'
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
    value = 1E-6
  [../]
  [./topy]
    type = PresetBC
    variable = y_disp
    boundary = front
    value = 1E-6
  [../]
  [./topz]
    type = PresetBC
    variable = z_disp
    boundary = front
    value = 1E-6
  [../]
[]

[AuxVariables]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
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
  [./yield_fcn]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zx
    index_i = 2
    index_j = 0
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
  [./yield_fcn_auxk]
    type = MaterialRealAux
    property = weak_plane_tensile_yield_function
    variable = yield_fcn
  [../]
[]

[Postprocessors]
  [./s_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
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
  [./str]
    type = TensorMechanicsHardeningConstant
    value = -1.0
  [../]
  [./wpt]
    type = TensorMechanicsPlasticWeakPlaneTensile
    tensile_strength = str
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
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 0
    plastic_models = wpt
    transverse_direction = '0 0 1'
    ep_plastic_tolerance = 1E-5
  [../]
[]


[Executioner]
  end_time = 1
  dt = 1
  type = Transient
[]


[Outputs]
  file_base = small_deform
  exodus = true
  [./csv]
    type = CSV
    [../]
[]

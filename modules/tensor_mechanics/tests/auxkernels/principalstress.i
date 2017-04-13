[Mesh]
  type = GeneratedMesh
  elem_type = HEX8
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin=0.0
  xmax=1.0
  ymin=0.0
  ymax=1.0
  zmin=0.0
  zmax=1.0
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Materials]
  [./fplastic]
    type = FiniteStrainPlasticMaterial
    block = 0
    yield_stress='0. 445. 0.05 610. 0.1 680. 0.38 810. 0.95 920. 2. 950.'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '2.827e5 1.21e5 1.21e5 2.827e5 1.21e5 2.827e5 0.808e5 0.808e5 0.808e5'
    fill_method = symmetric9
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[BCs]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./back]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./front]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = 't'
  [../]
  [./right]
    type = FunctionPresetBC
    variable = disp_y
    boundary = right
    function = '-0.5*t'
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./peeq]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_max]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_mid]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_min]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./peeq]
    type = RankTwoScalarAux
    rank_two_tensor = plastic_strain
    variable = peeq
    scalar_type = EquivalentPlasticStrain
  [../]
  [./stress_max]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = stress_max
    scalar_type = MaxPrincipal
  [../]
  [./stress_mid]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = stress_mid
    scalar_type = MidPrincipal
  [../]
  [./stress_min]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = stress_min
    scalar_type = MinPrincipal
  [../]
[]

[Postprocessors]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./peeq]
    type = ElementAverageValue
    variable = peeq
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./stress_max]
    type = ElementAverageValue
    variable = stress_max
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./stress_mid]
    type = ElementAverageValue
    variable = stress_mid
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./stress_min]
    type = ElementAverageValue
    variable = stress_min
    block = 'ANY_BLOCK_ID 0'
  [../]
[]

[Executioner]

  type = Transient

  dt=0.1
  dtmin=0.1
  dtmax=1
  end_time=1.0

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]


[Outputs]
  exodus = true
  csv = true
[]

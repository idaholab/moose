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

[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.0 0.0 0.0'
    new_boundary = 6
  [../]
  [./snode]
    type = AddExtraNodeset
    coord = '1.0 0.0 0.0'
    new_boundary = 7
  [../]
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

[Functions]
 [./topfunc]
   type = ParsedFunction
   value = 't'
 [../]
[]

[BCs]
  [./bottom3]
    type = PresetBC
    variable = disp_z
    boundary = 0
    value = 0.0
  [../]
  [./top]
    type = FunctionPresetBC
    variable = disp_z
    boundary = 5
    function = topfunc
  [../]
  [./corner1]
    type = PresetBC
    variable = disp_x
    boundary = 6
    value = 0.0
  [../]
  [./corner2]
    type = PresetBC
    variable = disp_y
    boundary = 6
    value = 0.0
  [../]
  [./corner3]
    type = PresetBC
    variable = disp_z
    boundary = 6
    value = 0.0
  [../]
  [./side1]
    type = PresetBC
    variable = disp_y
    boundary = 7
    value = 0.0
  [../]
  [./side2]
    type = PresetBC
    variable = disp_z
    boundary = 7
    value = 0.0
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
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./hydrostatic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./L2norm]
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
  [./vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vonmises
    scalar_type = VonMisesStress
  [../]
  [./hydrostatic]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = hydrostatic
    scalar_type = Hydrostatic
  [../]
  [./L2norm]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = L2norm
    scalar_type = L2norm
  [../]
  [./peeq]
    type = RankTwoScalarAux
    rank_two_tensor = plastic_strain
    variable = peeq
    scalar_type = EquivalentPlasticStrain
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
  [./vonmises]
    type = ElementAverageValue
    variable = vonmises
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./hydrostatic]
    type = ElementAverageValue
    variable = hydrostatic
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./L2norm]
    type = ElementAverageValue
    variable = L2norm
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

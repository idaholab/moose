

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 2
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
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
[]

[AuxVariables]
  # stresses
  [./e11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e12_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[AuxKernels]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = e11_aux
  [../]
  [./matl_e12]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 0
    index_j = 1
    variable = e12_aux
  [../]
  [./matl_e22]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 1
    index_j = 1
    variable = e22_aux
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1e6 0 0 1e6 0 1e6 .5e6 .5e6 .5e6'
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./eigenstrain]
    type = ComputeEigenstrain
    eigen_base = '0.1 0.05 0 0 0 0.01'
    prefactor = -1
    eigenstrain_name = eigenstrain
  [../]
[]

[BCs]
  [./bottom_y]
    type = PresetBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./left_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'


  nl_rel_tol = 1e-14
[]

[Outputs]
  file_base = applied_strain
  exodus = true
[]

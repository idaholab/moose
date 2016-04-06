[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
     [./InitialCondition]
      type = RandomIC
     [../]
  [../]

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
#stresses
  [./s11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s12_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s13_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s23_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./s33_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]

  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[AuxKernels]
  [./matl_s11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = s11_aux
  [../]

 [./matl_s12]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = s12_aux
  [../]

  [./matl_s13]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 2
    variable = s13_aux
  [../]

  [./matl_s22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = s22_aux
  [../]

  [./matl_s23]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 2
    variable = s23_aux
  [../]

  [./matl_s33]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = s33_aux
  [../]
[]

[Materials]
  # these materials replace the deprecated LinearElasticMaterial
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric9
    #reading C_11  C_12  C_13  C_22  C_23  C_33  C_44  C_55  C_66
    C_ijkl ='1.0e6  0.0   0.0 1.0e6  0.0  1.0e6 0.5e6 0.5e6 0.5e6'
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
[]

[BCs]
  [./bottom]
    type = PresetBC
    variable = diffused
    boundary = '1'
    value = 1
  [../]

  [./top]
    type = PresetBC
    variable = diffused
    boundary = '2'
    value = 0
  [../]

  [./disp_x_BC]
    type = PresetBC
    variable = disp_x
    boundary = '0 2'
    value = 0.5
  [../]

  [./disp_x_BC2]
    type = PresetBC
    variable = disp_x
    boundary = '1 3'
    value = 0.01
  [../]

  [./disp_y_BC]
    type = PresetBC
    variable = disp_y
    boundary = '0 2'
    value = 0.8
  [../]

  [./disp_y_BC2]
    type = PresetBC
    variable = disp_y
    boundary = '1 3'
    value = 0.02
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  nl_rel_tol = 1.e-12
[]

[Outputs]
  file_base = LinearElasticMaterial
  exodus = true
[]

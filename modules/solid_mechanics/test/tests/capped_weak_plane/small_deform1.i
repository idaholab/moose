# Elastic deformation.
# With Lame lambda=0 and Lame mu=1, applying the following
# deformation to the zmax surface of a unit cube:
# disp_x = 8*t
# disp_y = 6*t
# disp_z = t
# should yield stress:
# stress_xz = 8*t
# stress_xy = 6*t
# stress_zz = 2*t
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
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[BCs]
  [./bottomx]
    type = DirichletBC
    variable = disp_x
    boundary = back
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    variable = disp_y
    boundary = back
    value = 0.0
  [../]
  [./bottomz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]

  [./topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = front
    function = 8*t
  [../]
  [./topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = front
    function = 6*t
  [../]
  [./topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = t
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
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
  [./strainp_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_xz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strainp_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
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
  [./strainp_xx]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = strainp_xx
    index_i = 0
    index_j = 0
  [../]
  [./strainp_xy]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = strainp_xy
    index_i = 0
    index_j = 1
  [../]
  [./strainp_xz]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = strainp_xz
    index_i = 0
    index_j = 2
  [../]
  [./strainp_yy]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = strainp_yy
    index_i = 1
    index_j = 1
  [../]
  [./strainp_yz]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = strainp_yz
    index_i = 1
    index_j = 2
  [../]
  [./strainp_zz]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = strainp_zz
    index_i = 2
    index_j = 2
  [../]
[]

[Postprocessors]
  [./stress_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
  [./stress_xy]
    type = PointValue
    point = '0 0 0'
    variable = stress_xy
  [../]
  [./stress_xz]
    type = PointValue
    point = '0 0 0'
    variable = stress_xz
  [../]
  [./stress_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  [../]
  [./stress_yz]
    type = PointValue
    point = '0 0 0'
    variable = stress_yz
  [../]
  [./stress_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  [../]
  [./strainp_xx]
    type = PointValue
    point = '0 0 0'
    variable = strainp_xx
  [../]
  [./strainp_xy]
    type = PointValue
    point = '0 0 0'
    variable = strainp_xy
  [../]
  [./strainp_xz]
    type = PointValue
    point = '0 0 0'
    variable = strainp_xz
  [../]
  [./strainp_yy]
    type = PointValue
    point = '0 0 0'
    variable = strainp_yy
  [../]
  [./strainp_yz]
    type = PointValue
    point = '0 0 0'
    variable = strainp_yz
  [../]
  [./strainp_zz]
    type = PointValue
    point = '0 0 0'
    variable = strainp_zz
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '0 1'
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = ''
    perform_finite_strain_rotations = false
  [../]
[]

[Executioner]
  end_time = 2
  dt = 1
  type = Transient
[]

[Outputs]
  file_base = small_deform1
  csv = true
[]

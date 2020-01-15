# This test is designed to test the correct application of the Euler angle
# rotations to the elasticity tensor. The test uses values for the nine C_ijkl
# entries that correspond to the engineering notation placement:
#  e.g. C11 = 11e3, c12 = 12e3, c13 = 13e3, c22 = 22e3 ..... c66 = 66e3
#
# A rotation of (0, 90, 0) is applied to the 1x1x1 cube, such that the values of
# c12 and c13 switch, c22 and c33 switch, and c55 and c66 switch. Postprocessors
# are used to verify this switch (made simple with the value convention above)
# and to verify that the unrotated components along the x-axis remain constant.

[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [./lage_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./lage_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./pk2_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./lage_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./fp_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c12]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c13]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c22]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c23]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c33]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c44]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c55]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./c66]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./tdisp]
    type = ParsedFunction
    value = 0.01*t
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
  [../]
[]

[AuxKernels]
  [./lage_xx]
    type = RankTwoAux
    rank_two_tensor = lage
    variable = lage_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./lage_yy]
    type = RankTwoAux
    rank_two_tensor = lage
    variable = lage_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./pk2_yy]
    type = RankTwoAux
    variable = pk2_yy
    rank_two_tensor = pk2
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  [../]
  [./lage_zz]
    type = RankTwoAux
    rank_two_tensor = lage
    variable = lage_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]
  [./fp_yy]
    type = RankTwoAux
    variable = fp_yy
    rank_two_tensor = fp
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./c11]
    type = RankFourAux
    variable = c11
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    execute_on = timestep_end
  [../]
  [./c12]
    type = RankFourAux
    variable = c12
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    execute_on = timestep_end
  [../]
  [./c13]
    type = RankFourAux
    variable = c13
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 2
    index_l = 2
    execute_on = timestep_end
  [../]
  [./c22]
    type = RankFourAux
    variable = c22
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 1
    index_k = 1
    index_l = 1
    execute_on = timestep_end
  [../]
  [./c23]
    type = RankFourAux
    variable = c23
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 1
    index_k = 2
    index_l = 2
    execute_on = timestep_end
  [../]
  [./c33]
    type = RankFourAux
    variable = c33
    rank_four_tensor = elasticity_tensor
    index_i = 2
    index_j = 2
    index_k = 2
    index_l = 2
    execute_on = timestep_end
  [../]
  [./c44]
    type = RankFourAux
    variable = c44
    rank_four_tensor = elasticity_tensor
    index_i = 1
    index_j = 2
    index_k = 1
    index_l = 2
    execute_on = timestep_end
  [../]
  [./c55]
    type = RankFourAux
    variable = c55
    rank_four_tensor = elasticity_tensor
    index_i = 2
    index_j = 0
    index_k = 2
    index_l = 0
    execute_on = timestep_end
  [../]
  [./c66]
    type = RankFourAux
    variable = c66
    rank_four_tensor = elasticity_tensor
    index_i = 0
    index_j = 1
    index_k = 0
    index_l = 1
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = tdisp
  [../]
[]

[Materials]
  [./crysp]
    type = FiniteStrainCrystalPlasticity
    block = 0
    gtol = 1e-2
    slip_sys_file_name = input_slip_sys.txt
    nss = 12
    num_slip_sys_flowrate_props = 2 #Number of properties in a slip system
    flowprops = '1 4 0.001 0.1 5 8 0.001 0.1 9 12 0.001 0.1'
    hprops = '1.0 541.5 60.8 109.8 2.5'
    gprops = '1 4 60.8e3 5 8 60.8e3 9 12 60.8e3'
    tan_mod_type = exact
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '11e3 12e3 13e3 22e3 23e3 33e3 44e3 55e3 66e3'
    fill_method = symmetric9
    euler_angle_1 = 0.0
    euler_angle_2 = 90.0
    euler_angle_3 = 0.0
  [../]
[]

[Postprocessors]
  [./lage_xx]
    type = ElementAverageValue
    variable = lage_xx
  [../]
  [./pk2_yy]
    type = ElementAverageValue
    variable = pk2_yy
  [../]
  [./lage_yy]
    type = ElementAverageValue
    variable = lage_yy
  [../]
  [./lage_zz]
    type = ElementAverageValue
    variable = lage_zz
  [../]
  [./fp_yy]
    type = ElementAverageValue
    variable = fp_yy
  [../]
  [./c11]
    type = ElementAverageValue
    variable = c11
  [../]
  [./c12]
    type = ElementAverageValue
    variable = c12
  [../]
  [./c13]
    type = ElementAverageValue
    variable = c13
  [../]
  [./c22]
    type = ElementAverageValue
    variable = c22
  [../]
  [./c23]
    type = ElementAverageValue
    variable = c23
  [../]
  [./c33]
    type = ElementAverageValue
    variable = c33
  [../]
  [./c44]
    type = ElementAverageValue
    variable = c44
  [../]
  [./c55]
    type = ElementAverageValue
    variable = c55
  [../]
  [./c66]
    type = ElementAverageValue
    variable = c66
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  l_tol = 1e-3
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      1              lu            gmres     200'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  dtmax = 0.1
  dtmin = 1.0e-3
  dt = 0.05
  end_time = 0.5
[]

[Outputs]
  exodus = false
  csv = true
[]

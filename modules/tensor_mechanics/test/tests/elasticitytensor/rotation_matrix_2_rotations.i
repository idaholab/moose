# This input file is designed to rotate an elasticity tensor both with euler angles
# and a rotation matrix. The rotated tensor components should match between the
# two methods.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 1
[]

[AuxVariables]
  [./C1111_aux_matrix]  # C11
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C1122_aux_matrix]  # C12
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C1133_aux_matrix]  # C13
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C1112_aux_matrix]  # C16
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./C1111_aux_euler]  # C11
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C1122_aux_euler]  # C12
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C1133_aux_euler]  # C13
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./C1112_aux_euler]  # C16
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./matl_C1111_matrix]  # C11
    type = RankFourAux
    rank_four_tensor = rotation_matrix_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    variable = C1111_aux_matrix
    execute_on = initial
  [../]
  [./matl_C1122_matrix]  # C12
    type = RankFourAux
    rank_four_tensor = rotation_matrix_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    variable = C1122_aux_matrix
    execute_on = initial
  [../]
  [./matl_C1133_matrix]  # C13
    type = RankFourAux
    rank_four_tensor = rotation_matrix_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 2
    index_l = 2
    variable = C1133_aux_matrix
    execute_on = initial
  [../]
  [./matl_C1112_matrix]  # C16
    type = RankFourAux
    rank_four_tensor = rotation_matrix_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 1
    variable = C1112_aux_matrix
    execute_on = initial
  [../]

  [./matl_C1111_euler]  # C11
    type = RankFourAux
    rank_four_tensor = euler_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    variable = C1111_aux_euler
    execute_on = initial
  [../]
  [./matl_C1122_euler]  # C12
    type = RankFourAux
    rank_four_tensor = euler_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    variable = C1122_aux_euler
    execute_on = initial
  [../]
  [./matl_C1133_euler]  # C13
    type = RankFourAux
    rank_four_tensor = euler_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 2
    index_l = 2
    variable = C1133_aux_euler
    execute_on = initial
  [../]
  [./matl_C1112_euler]  # C16
    type = RankFourAux
    rank_four_tensor = euler_elasticity_tensor
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 1
    variable = C1112_aux_euler
    execute_on = initial
  [../]
[]

[Materials]
  [./elasticity_matrix]
    type = ComputeElasticityTensor
    block = 0
    base_name = 'rotation_matrix'
    fill_method = symmetric9
    C_ijkl = '1111 1122 1133 2222 2233 3333 2323 1313 1212'
    # rotation matrix for rotating a vector
    #   1. 45 degrees about z-axis
    #   2. ~54.7 degrees (arccos(1/sqrt(3)) radians) about x-axis
    # then taking the tranpose to give sample-to-crystal rotation,
    # ie. R*([0,0,1]) = [1,1,1], meaning the <001> direction of the sample
    # (or simulation) frame points along the <111> direction of the crystal
    rotation_matrix = '0.70710678  0.40824829  0.57735027
                      -0.70710678  0.40824829  0.57735027
                       0.         -0.81649658  0.57735027'
  [../]
  [./elasticity_euler]
    type = ComputeElasticityTensor
    block = 0
    base_name = 'euler'
    fill_method = symmetric9
    C_ijkl = '1111 1122 1133 2222 2233 3333 2323 1313 1212'
    # the angles here are the same as used to build the rotation matrix above because
    # we build the _transpose_ from euler angles in MOOSE, but we also transposed
    # the matrix for this example, so it goes back to the original;
    # the reversed order is due to the "extrinsic" convention used by MOOSE
    euler_angle_1 = 0.
    euler_angle_2 = 54.73561032
    euler_angle_3 = 45.
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  # corresponding values in "matrix" and "euler" postprocessors should match
  [./C11_matrix]
    type = ElementAverageValue
    variable = C1111_aux_matrix
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./C12_matrix]
    type = ElementAverageValue
    variable = C1122_aux_matrix
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./C13_matrix]
    type = ElementAverageValue
    variable = C1133_aux_matrix
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./C16_matrix]
    type = ElementAverageValue
    variable = C1112_aux_matrix
    execute_on = 'INITIAL TIMESTEP_END'
  [../]

  [./C11_euler]
    type = ElementAverageValue
    variable = C1111_aux_euler
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./C12_euler]
    type = ElementAverageValue
    variable = C1122_aux_euler
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./C13_euler]
    type = ElementAverageValue
    variable = C1133_aux_euler
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./C16_euler]
    type = ElementAverageValue
    variable = C1112_aux_euler
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Outputs]
  exodus = true
[]

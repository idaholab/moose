# Test for the stress and strain output for tapered shell elements.
# A tapered beam is represented with shell elements in XY plane
# having Young's Modulus of 210000 and poissons ratio of 0.3.
# The displacement in X direction is constrained in the left end and the
# displacement of center node of the left end is constrained in Y direction.
# A uniform displacement is applied at the right end.
# The problem is symmetric about Y-axis and the results are symmetric.

[Mesh]
  [input]
    type = FileMeshGenerator
    file = taperedmesh.e
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
[]

[AuxVariables]
  [stress_00]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_01]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_10]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_02]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_20]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_12]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_21]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_00]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_01]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_10]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_02]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_20]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_12]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_21]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    block = 1
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    block = 1
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    block = 1
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    block = 1
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    block = 1
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
[]

[AuxKernels]
  [stress_00]
    type = RankTwoAux
    variable = stress_00
    rank_two_tensor = global_stress_t_points_0
    index_i = 0
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [strain_00]
    type = RankTwoAux
    variable = strain_00
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 0
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [stress_11]
    type = RankTwoAux
    variable = stress_11
    rank_two_tensor = global_stress_t_points_0
    index_i = 1
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [strain_11]
    type = RankTwoAux
    variable = strain_11
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 1
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [stress_22]
    type = RankTwoAux
    variable = stress_22
    rank_two_tensor = global_stress_t_points_0
    index_i = 2
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [strain_22]
    type = RankTwoAux
    variable = strain_22
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 2
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [stress_01]
    type = RankTwoAux
    variable = stress_01
    rank_two_tensor = global_stress_t_points_0
    index_i = 0
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [strain_01]
    type = RankTwoAux
    variable = strain_01
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 0
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [stress_10]
    type = RankTwoAux
    variable = stress_10
    rank_two_tensor = global_stress_t_points_0
    index_i = 1
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [strain_10]
    type = RankTwoAux
    variable = strain_10
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 1
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [stress_02]
    type = RankTwoAux
    variable = stress_02
    rank_two_tensor = global_stress_t_points_0
    index_i = 0
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [strain_02]
    type = RankTwoAux
    variable = strain_02
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 0
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [stress_20]
    type = RankTwoAux
    variable = stress_20
    rank_two_tensor = global_stress_t_points_0
    index_i = 2
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [strain_20]
    type = RankTwoAux
    variable = strain_20
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 2
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [stress_12]
    type = RankTwoAux
    variable = stress_12
    rank_two_tensor = global_stress_t_points_0
    index_i = 1
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [strain_12]
    type = RankTwoAux
    variable = strain_12
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 1
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [stress_21]
    type = RankTwoAux
    variable = stress_21
    rank_two_tensor = global_stress_t_points_0
    index_i = 2
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [strain_21]
    type = RankTwoAux
    variable = strain_21
    rank_two_tensor = total_global_strain_t_points_0
    index_i = 2
    index_j = 1
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [BC_0]
    type = ADDirichletBC
    variable = disp_x
    value = 0.0
    boundary = '2' #left
  []
  [BC_1]
    type = ADDirichletBC
    variable = disp_y
    value = 0.0
    boundary = 10 #left_side_mid
  []
  [BC_2]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '3'
    function = displacement
  []
[]

[Functions]
  [displacement]
    type = PiecewiseLinear
    x = '0.0 1.0'
    y = '0.0 0.2'
  []
[]

[Materials]
  [stress]
    type = ADComputeShellStress
    block = 1
    through_thickness_order = SECOND
  []
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 210000
    poissons_ratio = 0.3
    block = 1
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    block = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.1
    through_thickness_order = SECOND
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  line_search = 'none'
  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-16

  dt = 1
  dtmin = 1
  end_time = 1
[]

[Outputs]
    exodus = true
[]

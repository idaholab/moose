# test for displacement of pinched cylinder with user-defined local vectors
# everything is similar to the pinch_cylinder_symm.i, except the local coordinates.
# in the original test the first local axis is '0 0 1'
# in this test, the first local vector is defined by the user : first_local_vector_ref='1 0 0'
# the given vector by the user is projected on the shell elements
# The rotational BCs are switched in order to get same results.

# Moreover, axiliary variables are added in this test to visualize local coordinates
# The local stresses, forces and bending moments are also calcualted
# The local stress_22 should be zero for all elements

[Mesh]
  [mesh]
    type = FileMeshGenerator
    file = cyl_sym_10x10.e
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
  [rot_x]
    order = FIRST
    family = LAGRANGE
  []
  [rot_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'CD AD'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'CD BC'
    value = 0.0
  []
  [simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'AB'
    value = 0.0
  []
  [simply_support_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = 'AB'
    value = 0.0
  []
  [simply_support_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'AD BC'
    value = 0.0
  []
[]

[DiracKernels]
  [point]
    type = ConstantPointSource
    variable = disp_x
    point = '1 0 1'
    value = -2.5 # P = 10
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
  [stress_02]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_12]
    order = CONSTANT
    family = MONOMIAL
  []
  [force_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [force_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [moment_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [moment_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [moment_12]
    order = CONSTANT
    family = MONOMIAL
  []
  [shear_12]
    order = CONSTANT
    family = MONOMIAL
  []
  [shear_13]
    order = CONSTANT
    family = MONOMIAL
  []
  [shear_23]
    order = CONSTANT
    family = MONOMIAL
  []
  [first_axis_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [first_axis_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [first_axis_z]
    order = CONSTANT
    family = MONOMIAL
  []
  [second_axis_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [second_axis_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [second_axis_z]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_axis_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_axis_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_axis_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_00]
    type = RankTwoAux
    variable = stress_00
    rank_two_tensor = local_stress_t_points_0
    index_i = 0
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [stress_11]
    type = RankTwoAux
    variable = stress_11
    rank_two_tensor = local_stress_t_points_0
    index_i = 1
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [stress_22]
    type = RankTwoAux
    variable = stress_22
    rank_two_tensor = local_stress_t_points_0
    index_i = 2
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [stress_01]
    type = RankTwoAux
    variable = stress_01
    rank_two_tensor = local_stress_t_points_0
    index_i = 0
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [stress_02]
    type = RankTwoAux
    variable = stress_02
    rank_two_tensor = local_stress_t_points_0
    index_i = 0
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [stress_12]
    type = RankTwoAux
    variable = stress_12
    rank_two_tensor = local_stress_t_points_0
    index_i = 1
    index_j = 2
    execute_on = TIMESTEP_END
  []

  [force_1]
    type = MaterialRealAux
    variable = force_1
    property = shell_force_1
    execute_on = TIMESTEP_END
  []
  [force_2]
    type = MaterialRealAux
    variable = force_2
    property = shell_force_2
    execute_on = TIMESTEP_END
  []
  [moment_11]
    type = MaterialRealAux
    variable = moment_11
    property = shell_moment_11
    execute_on = TIMESTEP_END
  []
  [moment_22]
    type = MaterialRealAux
    variable = moment_22
    property = shell_moment_22
    execute_on = TIMESTEP_END
  []

  [moment_12]
    type = MaterialRealAux
    variable = moment_12
    property = shell_moment_12
    execute_on = TIMESTEP_END
  []
  [shear_12]
    type = MaterialRealAux
    variable = shear_12
    property = shell_shear_12
    execute_on = TIMESTEP_END
  []
  [shear_13]
    type = MaterialRealAux
    variable = shear_13
    property = shell_shear_13
    execute_on = TIMESTEP_END
  []
  [shear_23]
    type = MaterialRealAux
    variable = shear_23
    property = shell_shear_23
    execute_on = TIMESTEP_END
  []
  [first_axis_x]
    type = MaterialRealVectorValueAux
    variable = first_axis_x
    property = first_local_vector
    component = 0
  []
  [first_axis_y]
    type = MaterialRealVectorValueAux
    variable = first_axis_y
    property = first_local_vector
    component = 1
  []
  [first_axis_z]
    type = MaterialRealVectorValueAux
    variable = first_axis_z
    property = first_local_vector
    component = 2
  []

  [second_axis_x]
    type = MaterialRealVectorValueAux
    variable = second_axis_x
    property = second_local_vector
    component = 0
  []
  [second_axis_y]
    type = MaterialRealVectorValueAux
    variable = second_axis_y
    property = second_local_vector
    component = 1
  []
  [second_axis_z]
    type = MaterialRealVectorValueAux
    variable = second_axis_z
    property = second_local_vector
    component = 2
  []

  [normal_axis_x]
    type = MaterialRealVectorValueAux
    variable = normal_axis_x
    property = normal_local_vector
    component = 0
  []
  [normal_axis_y]
    type = MaterialRealVectorValueAux
    variable = normal_axis_y
    property = normal_local_vector
    component = 1
  []
  [normal_axis_z]
    type = MaterialRealVectorValueAux
    variable = normal_axis_z
    property = normal_local_vector
    component = 2
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    block = '100'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    block = '100'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    block = '100'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    block = '100'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    block = '100'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = '100'
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    block = '100'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.01
    through_thickness_order = SECOND
    first_local_vector_ref = '1 0 0'
  []
  [stress]
    type = ADComputeShellStress
    block = '100'
    through_thickness_order = SECOND
  []
[]

[Postprocessors]
  [disp_x1]
    type = PointValue
    point = '1 0 1'
    variable = disp_x
  []
  [disp_y1]
    type = PointValue
    point = '1 0 1'
    variable = disp_y
  []
  [disp_x2]
    type = PointValue
    point = '0 1 1'
    variable = disp_x
  []
  [disp_y2]
    type = PointValue
    point = '0 1 1'
    variable = disp_y
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

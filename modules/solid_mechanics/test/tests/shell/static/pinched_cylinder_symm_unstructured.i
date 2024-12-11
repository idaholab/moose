# Test for displacement of pinched cylinder (similar to pinch_cyl_symm.i)
# This variant of the test is run with an unstructured mesh

[Mesh]
  [mesh]
    type = FileMeshGenerator
    file = pinched_cyl_10_10_unstructured.msh
  []
  [block_100]
    type = ParsedSubdomainMeshGenerator
    input = mesh
    combinatorial_geometry = 'x > -1.1 & x < 1.1 & y > -1.1 & y < 1.1 & z > -0.1 & z < 2.1'
    block_id = 100
  []
  [nodeset_1]
    type = BoundingBoxNodeSetGenerator
    input = block_100
    top_right = '1.1 1.1 0'
    bottom_left = '-1.1 -1.1 0'
    new_boundary = 'CD' #CD
  []
  [nodeset_2]
    type = BoundingBoxNodeSetGenerator
    input = nodeset_1
    top_right = '1.1 1.1 1.0'
    bottom_left = '-1.1 -1.1 1.0'
    new_boundary = 'AB' #AB
  []
  [nodeset_3]
    type = BoundingBoxNodeSetGenerator
    input = nodeset_2
    top_right = '0.02 1.1 1.0'
    bottom_left = '-0.1 0.98 0.0'
    new_boundary = 'AD' #AD
  []
  [nodeset_4]
    type = BoundingBoxNodeSetGenerator
    input = nodeset_3
    top_right = '1.1 0.02 1.0'
    bottom_left = '0.98 -0.1 0.0'
    new_boundary = 'BC' #BC
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

[AuxVariables]
  [stress_xx0]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xx1]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_xx0]
    type = RankTwoAux
    variable = stress_xx0
    rank_two_tensor = global_stress_t_points_0
    index_i = 0
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [stress_xx1]
    type = RankTwoAux
    variable = stress_xx1
    rank_two_tensor = global_stress_t_points_1
    index_i = 0
    index_j = 0
    execute_on = TIMESTEP_END
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
    boundary = 'AD BC'
    value = 0.0
  []
  [simply_support_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'AB'
    value = 0.0
  []
[]

[DiracKernels]
  [point1]
    type = ConstantPointSource
    variable = disp_x
    point = '1 0 1'
    value = -2.5 # P = 10
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
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'
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
    reference_first_local_direction = '0 0 1'
  []
  [stress]
    type = ADComputeShellStress
    block = '100'
    through_thickness_order = SECOND
  []
[]

[Postprocessors]
  [disp_z2]
    type = PointValue
    point = '1 0 1'
    variable = disp_x
  []
[]

[Outputs]
  exodus = true
[]

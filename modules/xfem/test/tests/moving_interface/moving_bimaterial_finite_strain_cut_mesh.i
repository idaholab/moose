# This test is for two layer materials with different youngs modulus with AD
# The global stress is determined by switching the stress based on level set values
# The material interface is marked by a level set function
# The two layer materials are glued together

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[XFEM]
  output_cut_plane = true
[]

[UserObjects]
  [cut]
    type = InterfaceMeshCut2DUserObject
    mesh_file = line.e
    interface_velocity_function = -1
    heal_always = true
  []
[]

[Mesh]
  use_displaced_mesh = true
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = 5
    elem_type = QUAD4
  []
  [left_bottom]
    type = ExtraNodesetGenerator
    new_boundary = 'left_bottom'
    coord = '0 0'
    input = generated_mesh
  []
  [left_top]
    type = ExtraNodesetGenerator
    new_boundary = 'left_top'
    coord = '0 5'
    input = left_bottom
  []
[]

# [Functions]
#   [ls_func]
#     type = ParsedFunction
#     expression = 'y-2.73+t'
#   []
# []

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [ls]
  []
  [a_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [a_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [a_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [b_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [b_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [b_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []

[]

[AuxKernels]
  # [ls_function]
  #   type = FunctionAux
  #   variable = ls
  #   function = ls_func
  # []
  [a_strain_xx]
    type = RankTwoAux
    variable = a_strain_xx
    rank_two_tensor = A_total_strain
    index_i = 0
    index_j = 0
  []
  [a_strain_yy]
    type = RankTwoAux
    variable = a_strain_yy
    rank_two_tensor = A_total_strain
    index_i = 1
    index_j = 1
  []
  [a_strain_xy]
    type = RankTwoAux
    variable = a_strain_xy
    rank_two_tensor = A_total_strain
    index_i = 0
    index_j = 1
  []
  [b_strain_xx]
    type = RankTwoAux
    variable = b_strain_xx
    rank_two_tensor = B_total_strain
    index_i = 0
    index_j = 0
  []
  [b_strain_yy]
    type = RankTwoAux
    variable = b_strain_yy
    rank_two_tensor = B_total_strain
    index_i = 1
    index_j = 1
  []
  [b_strain_xy]
    type = RankTwoAux
    variable = b_strain_xy
    rank_two_tensor = B_total_strain
    index_i = 0
    index_j = 1
  []
  [stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  []
  [stress_xy]
    type = RankTwoAux
    variable = stress_xy
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
  []
  [stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
  []
[]

[Kernels]
  [solid_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
    use_displaced_mesh = true
  []
  [solid_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
    use_displaced_mesh = true
  []
[]

[Constraints]
  [dispx_constraint]
    type = XFEMSingleVariableConstraint
    use_displaced_mesh = false
    variable = disp_x
    alpha = 1e8
    geometric_cut_userobject = 'level_set_cut_uo'
  []
  [dispy_constraint]
    type = XFEMSingleVariableConstraint
    use_displaced_mesh = false
    variable = disp_y
    alpha = 1e8
    geometric_cut_userobject = 'level_set_cut_uo'
  []
[]

[BCs]
  [bottomx]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [topx]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_x
    function = 0.03*t
  []
  [topy]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = '0.03*t'
  []
[]

[Materials]
  [elasticity_tensor_A]
    type = ComputeIsotropicElasticityTensor
    base_name = A
    youngs_modulus = 1e9
    poissons_ratio = 0.3
  []
  [strain_A]
    type = ComputeFiniteStrain
    base_name = A
  []
  [stress_A]
    type = ComputeFiniteStrainElasticStress
    base_name = A
  []
  [elasticity_tensor_B]
    type = ComputeIsotropicElasticityTensor
    base_name = B
    youngs_modulus = 1e7
    poissons_ratio = 0.3
  []
  [strain_B]
    type = ComputeFiniteStrain
    base_name = B
  []
  [stress_B]
    type = ComputeFiniteStrainElasticStress
    base_name = B
  []
  [combined_stress]
    type = LevelSetBiMaterialRankTwo
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = stress
  []
  [combined_jacob_mult]
    type = LevelSetBiMaterialRankFour
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = Jacobian_mult
  []
[]

[Postprocessors]
  [disp_x_norm]
    type = ElementL2Norm
    variable = disp_x
  []
  [disp_y_norm]
    type = ElementL2Norm
    variable = disp_y
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true

  # controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-13
  nl_abs_tol = 1e-50

  # time control
  start_time = 0.0
  dt = 0.1
  num_steps = 4

  max_xfem_update = 1
[]

[Outputs]
  print_linear_residuals = false
  exodus = true
[]

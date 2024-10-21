# This test is for two layer materials with different youngs modulus with AD
# The global stress is determined by switching the stress based on level set values
# The material interface is marked by a level set function
# The two layer materials are glued together
# This case is also meant to test for a bug in moving interfaces on displaced meshes
# It should fail during the healing step of the 2nd timestep if the bug is present.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./level_set_cut_uo]
    type = LevelSetCutUserObject
    level_set_var = ls
    heal_always = true
  [../]
[]

[Mesh]
  displacements = 'disp_x disp_y'
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmin = 0.0
    xmax = 5.
    ymin = 0.0
    ymax = 5.
    elem_type = QUAD4
  []
  [./left_bottom]
    type = ExtraNodesetGenerator
    new_boundary = 'left_bottom'
    coord = '0.0 0.0'
    input = generated_mesh
  [../]
  [./left_top]
    type = ExtraNodesetGenerator
    new_boundary = 'left_top'
    coord = '0.0 5.'
    input = left_bottom
  [../]
[]

[AuxVariables]
  [./ls]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./ls_function]
    type = FunctionAux
    variable = ls
    function = ls_func
  [../]
[]

[Functions]
  [./ls_func]
    type = ParsedFunction
    expression = 'y-3.153 + t'
  [../]
[]

[AuxVariables]
  [./a_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./a_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./a_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./b_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./b_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./b_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = SMALL
    use_automatic_differentiation = true
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_xy'
  [../]
[]

[AuxKernels]
  [./a_strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = A_total_strain
    index_i = 0
    index_j = 0
    variable = a_strain_xx
  [../]
  [./a_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = A_total_strain
    index_i = 1
    index_j = 1
    variable = a_strain_yy
  [../]
  [./a_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = A_total_strain
    index_i = 0
    index_j = 1
    variable = a_strain_xy
  [../]
  [./b_strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = B_total_strain
    index_i = 0
    index_j = 0
    variable = b_strain_xx
  [../]
  [./b_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = B_total_strain
    index_i = 1
    index_j = 1
    variable = b_strain_yy
  [../]
  [./b_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = B_total_strain
    index_i = 0
    index_j = 1
    variable = b_strain_xy
  [../]
[]

[Constraints]
  [./dispx_constraint]
    type = XFEMSingleVariableConstraint
    use_displaced_mesh = false
    variable = disp_x
    alpha = 1e8
    geometric_cut_userobject = 'level_set_cut_uo'
  [../]
  [./dispy_constraint]
    type = XFEMSingleVariableConstraint
    use_displaced_mesh = false
    variable = disp_y
    alpha = 1e8
    geometric_cut_userobject = 'level_set_cut_uo'
  [../]
[]

[BCs]
  [./bottomx]
    type = ADDirichletBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = ADDirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./topx]
    type = ADFunctionDirichletBC
    boundary = top
    variable = disp_x
    function = 0.03*t
  [../]
  [./topy]
    type = ADFunctionDirichletBC
    boundary = top
    variable = disp_y
    function = '0.03*t'
  [../]
[]

[Materials]
  [./elasticity_tensor_A]
    type = ADComputeIsotropicElasticityTensor
    base_name = A
    youngs_modulus = 1e9
    poissons_ratio = 0.3
  [../]
  [./strain_A]
    type = ADComputeSmallStrain
    base_name = A
    displacements = 'disp_x disp_y'
  [../]
  [./stress_A]
    type = ADComputeLinearElasticStress
    base_name = A
  [../]
  [./elasticity_tensor_B]
    type = ADComputeIsotropicElasticityTensor
    base_name = B
    youngs_modulus = 1e7
    poissons_ratio = 0.3
  [../]
  [./strain_B]
    type = ADComputeSmallStrain
    base_name = B
    displacements = 'disp_x disp_y'
  [../]
  [./stress_B]
    type = ADComputeLinearElasticStress
    base_name = B
  [../]
  [./combined_stress]
    type = ADLevelSetBiMaterialRankTwo
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = stress
  [../]
  [./combined_elasticity_tensor]
    type = ADLevelSetBiMaterialRankFour
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = elasticity_tensor
  [../]
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'bt'

# controls for linear iterations
  l_max_its = 20
  l_tol = 1e-8

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-50

# time control
  start_time = 0.0
  dt = 0.15
  num_steps = 3

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  file_base = moving_bimaterial_out
  perf_graph = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]

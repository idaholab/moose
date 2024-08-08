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
  [level_set_cut_uo]
    type = LevelSetCutUserObject
    level_set_var = ls
    heal_always = true
  []
  [esm]
    type = CutElementSubdomainModifier
    geometric_cut_userobject = level_set_cut_uo
    reinitialize_subdomains = '' #no reinitialization of variables or material properties
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
  [bottom]
    type = SubdomainBoundingBoxGenerator
    input = generated_mesh
    block_id = 0
    bottom_left = '0 0 0'
    top_right = '5 2.5 0'
  []
  [top]
    type = SubdomainBoundingBoxGenerator
    input = bottom
    block_id = 1
    bottom_left = '0 2.5 0'
    top_right = '5 5 0'
  []
[]

[Functions]
  [ls_func]
    type = ParsedFunction
    expression = 'y-2.73+t'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [ls]
  []
  [strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xy]
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
  [ls_function]
    type = FunctionAux
    variable = ls
    function = ls_func
  []
  [strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 0
  []
  [strain_yy]
    type = RankTwoAux
    variable = strain_yy
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
  []
  [strain_xy]
    type = RankTwoAux
    variable = strain_xy
    rank_two_tensor = total_strain
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
    block = 1
    youngs_modulus = 1e9
    poissons_ratio = 0.3
  []
  [strain_A]
    type = ComputeFiniteStrain
    block = 1
  []
  [stress_A]
    type = ComputeFiniteStrainElasticStress
    block = 1
  []
  [elasticity_tensor_B]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 1e7
    poissons_ratio = 0.3
  []
  [strain_B]
    type = ComputeFiniteStrain
    block = 0
  []
  [stress_B]
    type = ComputeFiniteStrainElasticStress
    block = 0
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

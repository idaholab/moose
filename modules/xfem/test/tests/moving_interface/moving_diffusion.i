[GlobalParams]
  order = FIRST
  family = LAGRANGE
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
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 3
  xmin = 0.0
  xmax = 1
  ymin = 0.0
  ymax = 1
  elem_type = QUAD4
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

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./ls_func]
    type = ParsedFunction
    expression = 'x-0.76+0.21*t'
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    variable = u
    diffusivity = diffusion_coefficient
  [../]
  [./time_deriv]
    type = TimeDerivative
    variable = u
  [../]
[]

[Constraints]
  [./u_constraint]
    type = XFEMSingleVariableConstraint
    use_displaced_mesh = false
    variable = u
    jump = 0
    use_penalty = true
    alpha = 1e5
    geometric_cut_userobject = 'level_set_cut_uo'
  [../]
[]

[BCs]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./diffusivity_A]
    type = GenericConstantMaterial
    prop_names = A_diffusion_coefficient
    prop_values = 5
  [../]

  [./diffusivity_B]
    type = GenericConstantMaterial
    prop_names = B_diffusion_coefficient
    prop_values = 0.5
  [../]

  [./diff_combined]
    type = LevelSetBiMaterialReal
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = diffusion_coefficient
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  l_max_its = 20
  l_tol = 1e-3
  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5

  start_time = 0.0
  dt = 1
  end_time = 2

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  perf_graph = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]

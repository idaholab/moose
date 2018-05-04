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
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 11
  ny = 5
  xmin = 0.0
  xmax = 5.
  ymin = 0.0
  ymax = 10
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
    value = 'x-4.5'
  [../]
[]

[Kernels]
  [./diff]
    type = XFEMDiffusion
    variable = u
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
    jump = -0.3
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
    prop_values = 100
  [../]

  [./diffusivity_B]
    type = GenericConstantMaterial
    prop_names = B_diffusion_coefficient
    prop_values = 0.1
  [../]

  [./diff_combined]
    type = LevelSetBiMaterialDiffusion
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'


  # petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  # petsc_options_value = 'lu     superlu_dist'

  line_search = 'bt'

  #[./Predictor]
  #  type = SimplePredictor
  #  scale = 1.0
  #[../]

# controls for linear iterations
  l_max_its = 20
  l_tol = 1e-3

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5

# time control
  start_time = 0.0
  dt = 1
  end_time = 20.0
  num_steps = 5000

  max_xfem_update = 1
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  csv = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]


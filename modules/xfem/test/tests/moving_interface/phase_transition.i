[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 11
  ny = 1
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./velocity]
    type = XFEMPhaseTransitionMovingInterfaceVelocity
    diffusivity_at_positive_level_set = 5
    diffusivity_at_negative_level_set = 1
    equilibrium_concentration_jump = 1
    value_at_interface_uo = value_uo
  [../]
  [./value_uo]
    type = PointValueAtXFEMInterface
    variable = 'u'
    geometric_cut_userobject = 'moving_line_segments'
    execute_on = 'nonlinear'
    level_set_var = ls
  [../]
  [./moving_line_segments]
    type = MovingLineSegmentCutSetUserObject
    cut_data = '0.5 0 0.5 1.0 0 0'
    heal_always = true
    interface_velocity = velocity
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./ic_u]
    type = FunctionIC
    variable = u
    function = 'if(x<0.51, 2, 1)'
  [../]
[]

[AuxVariables]
  [./ls]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Constraints]
  [./u_constraint]
    type = XFEMEqualValueAtInterface
    geometric_cut_userobject = 'moving_line_segments'
    use_displaced_mesh = false
    variable = u
    value = 2
    alpha = 1e5
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    variable = u
    D_name = 'diffusion_coefficient'
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxKernels]
  [./ls]
    type = LineSegmentLevelSetAux
    line_segment_cut_set_user_object = 'moving_line_segments'
    variable = ls
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
    prop_values = 1
  [../]
  [./diff_combined]
    type = LevelSetBiMaterialReal
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = diffusion_coefficient
  [../]
[]

[BCs]
# Define boundary conditions
  [./left_u]
    type = DirichletBC
    variable = u
    value = 2
    boundary = 3
  [../]

  [./right_u]
    type = NeumannBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'

  l_tol = 1e-3
  nl_max_its = 15
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-11

  start_time = 0.0
  dt = 0.01
  num_steps = 4
  max_xfem_update = 1
[]


[Outputs]
  execute_on = timestep_end
  exodus = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
  csv = true
[]

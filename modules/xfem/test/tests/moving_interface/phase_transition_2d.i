[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 11
    ny = 1
    xmin = 0.0
    xmax = 20.0
    ymin = 0.0
    ymax = 5.0
    elem_type = QUAD4
  []
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [velocity]
    type = XFEMPhaseTransitionMovingInterfaceVelocity
    diffusivity_at_positive_level_set = 5
    diffusivity_at_negative_level_set = 1
    equilibrium_concentration_jump = 1
    value_at_interface_uo = value_uo
  []
  [value_uo]
    type = NodeValueAtXFEMInterface
    variable = 'u'
    interface_mesh_cut_userobject = 'cut_mesh'
    execute_on = TIMESTEP_END
    level_set_var = ls
  []
  [cut_mesh]
    type = InterfaceMeshCut2DUserObject
    mesh_file = flat_interface_1d.e
    interface_velocity_uo = velocity
    heal_always = true
  []
[]

[Variables]
  [u]
  []
[]

[ICs]
  [ic_u]
    type = FunctionIC
    variable = u
    function = 'if(x<5.01, 2, 1)'
  []
[]

[AuxVariables]
  [ls]
    order = FIRST
    family = LAGRANGE
  []
[]

[Constraints]
  [u_constraint]
    type = XFEMEqualValueAtInterface
    geometric_cut_userobject = 'cut_mesh'
    use_displaced_mesh = false
    variable = u
    value = 2
    alpha = 1e6
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = diffusion_coefficient
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[AuxKernels]
  [ls]
    type = MeshCutLevelSetAux
    mesh_cut_user_object = cut_mesh
    variable = ls
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Materials]
  [diffusivity_A]
    type = GenericConstantMaterial
    prop_names = A_diffusion_coefficient
    prop_values = 5
  []
  [diffusivity_B]
    type = GenericConstantMaterial
    prop_names = B_diffusion_coefficient
    prop_values = 1
  []
  [diff_combined]
    type = LevelSetBiMaterialReal
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = ls
    prop_name = diffusion_coefficient
  []
[]

[BCs]
  # Define boundary conditions
  [left_u]
    type = DirichletBC
    variable = u
    value = 2
    boundary = left
  []

  [right_u]
    type = NeumannBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'

  l_tol = 1e-3
  nl_max_its = 15
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-9

  start_time = 0.0
  dt = 1
  num_steps = 5
  max_xfem_update = 1
[]

[Outputs]
  execute_on = timestep_end
  exodus = true
  perf_graph = true
[]

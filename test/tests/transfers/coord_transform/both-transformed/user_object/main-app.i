[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = -1
  ymax = 0
  nx = 10
  ny = 10
  # Quarter turn around Z axis
  alpha_rotation = -90
  # Flips around Y axis
  # beta_rotation = -180
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [from_sub_app_var][]
  [from_sub_app_var_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = CoupledForce
    variable = u
    v = from_sub_app_var
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  verbose = true
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [main_uo]
    type = LayeredAverage
    direction = x
    num_layers = 5
    variable = u
  []
[]

[MultiApps]
  [sub_app]
    # Shift is offset by sub-app mesh + rotations
    # positions = '1 0 0.0'
    type = FullSolveMultiApp
    input_files = sub-app.i
    app_type = MooseTestApp
    bounding_box_padding = '0.25 0.25 0'
    bounding_box_inflation = 0
    use_displaced_mesh = true
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [layered_transfer_to_sub_app]
    type = MultiAppUserObjectTransfer
    user_object = main_uo
    variable = sub_app_var
    to_multi_app = sub_app
    displaced_target_mesh = true
  []
  [layered_transfer_to_sub_app_elem]
    type = MultiAppUserObjectTransfer
    user_object = main_uo
    variable = sub_app_var_elem
    to_multi_app = sub_app
    displaced_target_mesh = true
  []

  [layered_transfer_from_sub_app]
    type = MultiAppUserObjectTransfer
    user_object = sub_app_uo
    variable = from_sub_app_var
    from_multi_app = sub_app
    # displaced_source_mesh = true
  []
  [layered_transfer_from_sub_app_elem]
    type = MultiAppUserObjectTransfer
    user_object = sub_app_uo
    variable = from_sub_app_var_elem
    from_multi_app = sub_app
    # displaced_source_mesh = true
  []
[]

# This does a dummy diffusion solve in 3D space, then computes a layered average
# in the z direction. Those values are transferred into a sub-app that has 1D mesh
# in the z-direction (the mesh was displaced so that it is aligned in such a way).
# The sub-app also does a dummy diffusion solve and then computes layered average
# in the z-direction. Those value are transferred back to the parent app.
#
# Physically the 1D sub-app is placed in the center of the 3D mesh and is oriented
# in the z-direction.  The bounding box of the sub-app is expanded such that it
# contains the 4 central elements of the 3D mesh (i.e. the values are transferred
# only into a part of parent mesh)

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 10
  # The MultiAppUserObjectTransfer object only works with ReplicatedMesh
  parallel_type = replicated
[]

[AuxVariables]
  [./from_sub_app_var]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[UserObjects]
  [main_uo]
    type = LayeredAverage
    direction = z
    num_layers = 10
    variable = u
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [td]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = front
    value = -1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = back
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 5

  solve_type = 'NEWTON'
  l_tol = 1e-8
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  execute_on = final
[]

[MultiApps]
  [sub_app]
    positions = '0.5 0.5 0.0'
    type = TransientMultiApp
    input_files = 3d_1d_sub.i
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
  [layered_transfer_from_sub_app]
    type = MultiAppUserObjectTransfer
    user_object = sub_app_uo
    variable = from_sub_app_var
    from_multi_app = sub_app
    displaced_source_mesh = true
  []
[]

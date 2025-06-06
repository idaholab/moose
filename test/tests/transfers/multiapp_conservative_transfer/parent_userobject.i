[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 20
  ny = 20
  nz = 20
  # The MultiAppUserObjectTransfer object only works with ReplicatedMesh
  parallel_type = replicated
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [multi_layered_average]
  []
  [element_multi_layered_average]
    order = CONSTANT
    family = MONOMIAL
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
  type = Transient
  num_steps = 1
  dt = 0.001 # This will be constrained by the multiapp

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  l_tol = 1e-8
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  csv = true
[]

[VectorPostprocessors]
  [to_nearest_point]
    type = NearestPointIntegralVariablePostprocessor
    variable = multi_layered_average
    points = '0.3 0.1 0.3 0.7 0.1 0.3'
    execute_on = 'transfer'
  []

  [to_nearest_point_element]
    type = NearestPointIntegralVariablePostprocessor
    variable = element_multi_layered_average
    points = '0.3 0.1 0.3 0.7 0.1 0.3'
    execute_on = 'transfer'
  []
[]

[MultiApps]
  [sub_app]
    positions = '0.3 0.1 0.3 0.7 0.1 0.3'
    type = TransientMultiApp
    input_files = sub_userobject.i
    app_type = MooseTestApp
  []
[]

[Transfers]
  [layered_transfer]
    source_user_object = layered_average
    variable = multi_layered_average
    type = MultiAppGeneralFieldUserObjectTransfer
    from_multi_app = sub_app
    skip_coordinate_collapsing = true

    from_postprocessors_to_be_preserved = 'from_postprocessor'
    to_postprocessors_to_be_preserved = 'to_nearest_point'
    # Test features non-overlapping meshes
    error_on_miss = false
  []
  [element_layered_transfer]
    source_user_object = layered_average
    variable = element_multi_layered_average
    type = MultiAppGeneralFieldUserObjectTransfer
    from_multi_app = sub_app
    skip_coordinate_collapsing = true

    from_postprocessors_to_be_preserved = 'from_postprocessor'
    to_postprocessors_to_be_preserved = 'to_nearest_point_element'
    # Test features non-overlapping meshes
    error_on_miss = false
  []
[]

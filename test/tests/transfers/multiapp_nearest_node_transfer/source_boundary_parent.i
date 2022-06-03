[Mesh]
  [drmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 30
    ny = 30
    xmax = 2
    elem_type = QUAD4
    partition = square
  []
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [from_sub][]
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 10
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    input_files = 'source_boundary_sub.i'
    positions = '-1.0 0.0 0.0
                  2. 0.0 0.0'
    output_in_position = true
    cli_args='BCs/right/value="1" BCs/right/value="10"'
  []
[]

[Transfers]
  [source_boundary]
    type = MultiAppNearestNodeTransfer
    source_variable = u
    from_multi_app = sub
    variable = from_sub
    source_boundary = 'right'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-6
[]

[Outputs]
  exodus = true
[]

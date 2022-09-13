[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 6
  ny = 6
  nz = 3
  xmin = -1
  ymin = -1
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
  [force]
    type = BodyForce
    function = 'x^2 + y^2 + z'
    variable = u
  []
[]

[AuxVariables]
  [v][]
[]

[BCs]
  [square]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = '2d-rz.i'
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = v
    execute_on = 'timestep_end'
    skip_coordinate_collapsing = false
  []
  [from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = v
    execute_on = 'timestep_end'
    skip_coordinate_collapsing = false
  []
[]

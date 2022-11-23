[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 1
  xmax = 3
  nx = 20
  ny = 10
  length_unit = '5*m'
  alpha_rotation = 90
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [v][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = CoupledForce
    variable = u
    v = v
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
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0 5 0'
    input_files = 'transform-main-sub-app.i'
    execute_on = 'timestep_begin'
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = v
    variable = v
    execute_on = 'timestep_begin'
  []
[]

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Positions]
  [multiapp]
    type = MultiAppPositions
    multiapps = 'm1 m2 m3'
  []
[]

[MultiApps]
  [m1]
    type = CentroidMultiApp
    input_files = 'multiapp_positions.i'
    cli_args = "MultiApps/active='';Positions/active=''"
  []
  [m2]
    type = FullSolveMultiApp
    input_files = 'multiapp_positions.i'
    cli_args = "MultiApps/active='';Positions/active=''"
  []
  [m3]
    type = FullSolveMultiApp
    input_files = 'multiapp_positions.i'
    positions = '0.2 0.4 1
                 0 0 6'
    cli_args = "MultiApps/active='';Positions/active=''"
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]

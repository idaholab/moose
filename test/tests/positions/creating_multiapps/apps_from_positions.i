[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[AuxVariables]
  [u]
  []
[]

[Positions]
  [input]
    type = InputPositions
    positions = '0 1 0
                 1 0 2'
    outputs = none
  []
  [file]
    type = FilePositions
    files = '../../multiapps/positions_from_file/positions.txt'
    outputs = none
  []
  [reporter_forward]
    type = ReporterPositions
    reporters = 'file/positions_1d'
    outputs = none
  []
  [mesh]
    type = ElementCentroidPositions
    outputs = none
  []

  [results_m1]
    type = MultiAppPositions
    multiapps = 'm1'
  []
  [results_m2]
    type = MultiAppPositions
    multiapps = 'm2'
  []
  [results_m3]
    type = MultiAppPositions
    multiapps = 'm3'
  []
  [results_m4]
    type = MultiAppPositions
    multiapps = 'm4'
  []
[]

[MultiApps]
  [m1]
    type = FullSolveMultiApp
    input_files = 'apps_from_positions.i'
    cli_args = "MultiApps/active='';Positions/active='';Outputs/active=''"
    positions_objects = input
  []
  [m2]
    type = FullSolveMultiApp
    input_files = 'apps_from_positions.i'
    cli_args = "MultiApps/active='';Positions/active='';Outputs/active=''"
    positions_objects = 'input file'
  []
  # Those Positions are executed too late
  [m3]
    type = TransientMultiApp
    input_files = 'apps_from_positions.i'
    cli_args = "MultiApps/active='';Positions/active='';Outputs/active=''"
    positions_objects = reporter_forward
  []
  [m4]
    type = TransientMultiApp
    input_files = 'apps_from_positions.i'
    cli_args = "MultiApps/active='';Positions/active='';Outputs/active=''"
    positions_objects = mesh
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

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Times]
  [exodus]
    type = ExodusFileTimes
    files = '../executioners/transient_sync_time/gold/out_tio.e ../executioners/transient_sync_time/gold/out.e'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]

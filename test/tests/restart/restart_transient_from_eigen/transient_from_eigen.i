[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 10
[]

[Problem]
  restart_file_base = eigen_out_cp/LATEST
  # no need to solve, we just want to make sure we have the state
  solve = false
[]

[Variables/u]
[]

[Executioner]
  type = Transient
  # solve to timestep 1, and then just compare timestep 1
  start_time = -1
  num_steps = 2
[]

[Outputs]
  exodus = true
[]

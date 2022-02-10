[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]

[MultiApps]
  [full_solve]
    type = FullSolveMultiApp
    input_files = sub.i
    no_backup_and_restore = false
  [../]
[]

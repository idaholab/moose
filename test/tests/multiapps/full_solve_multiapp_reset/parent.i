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
  num_steps = 2
[]

[MultiApps]
  [full_solve]
    type = FullSolveMultiApp
    execute_on = initial
    positions = '0 0 0'
    input_files = sub.i
    reset_apps = '0'
    reset_time = 1
  [../]
[]

[Mesh]
  type = GeneratedMesh
  nx = 4
  ny = 4
  nz = 4
[../]

[Variables]
  [./c]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
    [../]
  [../]
[]

[UserObjects]
  [./test]
    type = PeriodicNodeMapTester
    v = c
    execute_on = 'INITIAL'
  [../]
[]

[Executioner]
  type = Steady
  nl_abs_step_tol = 1e-9
[]

[Outputs]
  perf_graph = true
[]

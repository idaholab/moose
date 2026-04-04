[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
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
  [nan_test]
    type = NanKernel
    variable = u
    timestep_to_nan = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

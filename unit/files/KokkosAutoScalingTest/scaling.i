[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = KokkosDiffusion
    variable = u
  []
[]

[Executioner]
  type = Steady
  automatic_scaling = true
  resid_vs_jac_scaling_param = 0
[]

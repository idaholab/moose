[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
[]

[UserObjects]
  [exception]
    type = ExceptionElementUserObject
    rank = 1
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Steady
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
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

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [from_parent]
    type = Receiver
    default = -1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

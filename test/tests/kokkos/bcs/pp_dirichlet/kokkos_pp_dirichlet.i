# Diffusion on the unit square with u = 0 on the left and u equal to a postprocessor value on the
# right, giving u = 2*x.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosPostprocessorDirichletBC
    variable = u
    boundary = right
    postprocessor = right_value
  []
[]

[Postprocessors]
  [right_value]
    type = Receiver
    default = 2
    execute_on = 'initial linear'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]

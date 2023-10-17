[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Problem]
  linear_sys_names = 'u_sys'
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    nl_sys = 'u'
  []
[]

[FVKernels]
  [force]
    type = FVBodyForce
    variable = u
  []
[]

[Executioner]
  type = LinearPicardSolve
[]

[Outputs]
  exodus = true
[]

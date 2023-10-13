[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Problem]
  nl_sys_names = 'u v'
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

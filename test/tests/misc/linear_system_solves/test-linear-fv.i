[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Problem]
  linear_sys_names = 'u_sys'
  error_on_jacobian_nonzero_reallocation = true
  solve = false
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    linear_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [reaction]
    type = LinearFVReactionKernel
    variable = u
    coeff = 5.0
  []
  [source]
    type = LinearFVSourceKernel
    variable = u
    source_density = 1.0
  []
[]

[Executioner]
  type = LinearPicardSteady
  linear_sys_to_solve = u_sys
[]

[Outputs]
  exodus = true
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
    [InitialCondition]
      type = FunctionIC
      function = '5*x+y'
    []
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [react]
    type = Reaction
    variable = u
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-15
  nl_rel_tol = 1e-12
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]

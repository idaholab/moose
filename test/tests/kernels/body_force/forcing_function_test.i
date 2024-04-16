[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  uniform_refine = 4
[]

[Variables]
  [u]
  []
[]

[Functions]
  [forcing_func]
    type = ParsedFunction
    expression = alpha*alpha*pi*pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '16'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [forcing]
    type = BodyForce
    variable = u
    function = forcing_func
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

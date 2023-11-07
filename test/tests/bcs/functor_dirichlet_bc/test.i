[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 32
    ny = 32
  []
[]

[Variables]
  [u]
  []
[]

[Functions]
  [ff_1]
    type = ParsedFunction
    expression = alpha*alpha*pi
    symbol_names = 'alpha'
    symbol_values = '16'
  []

  [ff_2]
    type = ParsedFunction
    expression = pi*sin(alpha*pi*x)
    symbol_names = 'alpha'
    symbol_values = '16'
  []

  [forcing_func]
    type = CompositeFunction
    functions = 'ff_1 ff_2'
  []

  [bc_func]
    type = ParsedFunction
    expression = sin(alpha*pi*x)
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
  [all]
    type = FunctorDirichletBC
    variable = u
    boundary = 'left right'
    functor = bc_func
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

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Functions]
  [parsed]
    type = KokkosParsedFunction
    expression = 'if (x > y, 0.1*x, 0.1*y) + const + t + t_pp'
    symbol_names = 'const t_pp'
    symbol_values = '1 time'
    print_rpn = true
  []
[]

[Kernels]
  [diff]
    type = KokkosParsedFuncCoefDiffusion
    variable = u
    coef = parsed
  []
  [time]
    type = KokkosTimeDerivative
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
    type = KokkosNeumannBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [time]
    type = ParsedPostprocessor
    expression = 't'
    use_t = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

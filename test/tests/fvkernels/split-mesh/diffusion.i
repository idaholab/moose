[Variables]
  [v]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = 1
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [exact]
    type = FVFunctorDirichletBC
    boundary = 'left right top bottom'
    functor = 0.0
    variable = v
  []
[]

[Functions]
  [forcing]
    type = ParsedFunction
    expression = '-d1*18*y'
    symbol_names = 'd1'
    symbol_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

# [Mesh]
#   [fmg]
#     type = FileMeshGenerator
#     file = 'foo.cpr'
#   []
#   parallel_type = distributed
# []

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
    coeff_interp_method = average
    variable_interp_method = skewness-corrected
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
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
  csv = true
  exodus = true
[]

[Debug]
  show_actions = true
  show_execution_order = ALWAYS
[]

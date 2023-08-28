S = 10
D = 10

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 10
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [v_aux]
  []
[]

[Kernels]
  [diffusion_v]
    type = MatDiffusion
    variable = v
    diffusivity = D_v
  []
  [source_v]
    type = BodyForce
    variable = v
    value = 1.0
  []
[]

[AuxKernels]
  [func_aux]
    type = FunctionAux
    variable = v_aux
    function = v_aux_func
  []
[]

[Functions]
  [v_aux_func]
    type = ParsedFunction
    expression = 'S * x + D'
    symbol_names = 'S D'
    symbol_values = '${S} ${D}'
  []
[]

[Materials]
  [diffusivity_v]
    type = GenericConstantMaterial
    prop_names = D_v
    prop_values = 4.0
  []
[]

[BCs]
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Reporters]
  [solution_storage]
    type = SolutionContainer
    execute_on = 'FINAL'
  []
  [solution_storage_aux]
    type = SolutionContainer
    execute_on = 'FINAL'
    system = aux
  []
[]

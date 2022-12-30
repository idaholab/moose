a = 1.1

[Mesh]
  coord_type = 'RZ'
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  []
[]

[FVKernels]
  [advection]
    type = FVAdvection
    variable = v
    velocity = '${a} ${a} 0'
    advected_interp_method = 'average'
  []
  [reaction]
    type = FVReaction
    variable = v
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [left_u]
    type = FVFunctionDirichletBC
    boundary = 'left bottom'
    function = 'exact'
    variable = v
  []
  [right_u]
    type = FVConstantScalarOutflowBC
    variable = v
    velocity = '${a} ${a} 0'
    boundary = 'right top'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'sin(x)*cos(y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '-a*sin(x)*sin(y) + sin(x)*cos(y) + (x*a*cos(x)*cos(y) + a*sin(x)*cos(y))/x'
    symbol_names = 'a'
    symbol_values = '${a}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type -sub_pc_type'
  petsc_options_value = 'asm      NONZERO                   lu'
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    variable = v
    function = exact
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]

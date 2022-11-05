a=1.1
diff=1.1

[Mesh]
  [./gen_mesh]
    type = FileMeshGenerator
    file = skewed.msh
  [../]
[]


[Variables]
  [./v]
    initial_condition = 1
    type = MooseVariableFVReal
    face_interp_method = 'skewness-corrected'
  [../]
[]

[FVKernels]
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = ${diff}
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [exact]
    type = FVFunctionDirichletBC
    boundary = 'left right top bottom'
    function = 'exact'
    variable = v
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'sin(x)*cos(y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '2*diff*sin(x)*cos(y)'
    symbol_names = 'a diff'
    symbol_values = '${a} ${diff}'
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
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    variable = v
    function = exact
    outputs = 'console csv'
  [../]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
  []
[]

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
    cache_face_gradients = false
    cache_face_values = true
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
    value = 'sin(x)*cos(y)'
  []
  [forcing]
    type = ParsedFunction
    value = '2*diff*sin(x)*cos(y)'
    vars = 'a diff'
    vals = '${a} ${diff}'
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

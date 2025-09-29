[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]
[Functions]
  [dts]
    type = ParsedFunction
    expression = t^2
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 1
  [TimeStepper]
    type = FunctionDT
    function = dts
    min_dt = 1
  []
[]

[Reporters]
  [vecs]
    type = ConstantReporter
    real_vector_names = 'vec_a vec_b vec_c vec_d'
    real_vector_values = '1 2 3; 10 20 30; 100 10 1; 1 2 3 4'
    real_names = 'a b c'
    real_values = '1 10 100'
    outputs = none
  []
  [vectorOperation]
    type = ParsedVectorReporter
    name = inner
    vector_reporter_names = 'vecs/vec_a vecs/vec_b vecs/vec_c'
    vector_reporter_symbols = 'vec_a vec_b vec_c'
    scalar_reporter_names = 'vecs/a dt/value'
    scalar_reporter_symbols = 'a dt'
    constant_names = 'constant1 constant2'
    constant_expressions = '10 20'
    expression = 'vec_a+vec_b+vec_c+constant1+constant2+a+dt'
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  csv = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./dummy]
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Functions]
  [./input]
    type = CoarsenedPiecewiseLinear
    data_file = input.csv
    format = columns
    epsilon = 0.1
    x_scale = 0.03
  [../]
[]

[VectorPostprocessors]
  [./F]
    type = PiecewiseFunctionTabulate
    function = input
    execute_on = INITIAL
    outputs = vpp
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [./vpp]
    type = CSV
    execute_vector_postprocessors_on = INITIAL
  [../]
[]

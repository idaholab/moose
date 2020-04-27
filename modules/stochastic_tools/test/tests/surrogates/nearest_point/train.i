[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 1 10
                          0 1 10
                          0 1 10'
  []
[]

[VectorPostprocessors]
  [values]
    type = GFunction
    sampler = sample
    q_vector = '0 0 0'
    execute_on = INITIAL
    outputs = none
  []
[]

[Trainers]
  [train]
    type = NearestPointTrainer
    sampler = sample
    results_vpp = values
    results_vector = g_values
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'train'
    execute_on = FINAL
  []
[]

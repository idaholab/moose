[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
  []
[]

[Testing/LotsOfDiffusion/lots]
  number = 10
  array = true
  n_components = 4
  diffusion_coefficients = '1 1 1 1'
  add_reaction = true
[]

[Functions]
  [dc]
    type = ParsedFunction
    expression = t+1
  []
[]

[Controls]
  [setdc]
    type = RealVectorFunctionControl
    function = dc
    parameter = Testing/LotsOfDiffusion/lots/diffusion_coefficients
    execute_on = timestep_begin
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Outputs]
  exodus = true
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 32
    ny = 32
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./exact_func]
    type = ParsedFunction
    expression = x*x
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = 2
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = FunctionNeumannBC
    function = x
    variable = u
    boundary = right
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = neumannbc_out
  exodus = true
[]

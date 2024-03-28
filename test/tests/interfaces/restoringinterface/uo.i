[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [react]
    type = Reaction
    variable = u
  []
[]

[Postprocessors]
  [add_one]
    type = AddOnePostprocessor
  []
  [t]
    type = TimePostprocessor
  []
  [dt]
    type = TimestepSize
  []
[]

[UserObjects]
  [fail]
    type = Terminator
    expression = 't=5 & dt=1'
    fail_mode = SOFT
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 10
[]

[Outputs]
  csv = true
[]

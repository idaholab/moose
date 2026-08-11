[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[Variables]
  [u]
  []
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[Postprocessors]
  [from_parent]
    type = Receiver
    default = 0
  []
  [to_parent]
    type = ParsedPostprocessor
    expression = 'from_parent + 1'
    pp_names = 'from_parent'
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = MFEMSteady
[]

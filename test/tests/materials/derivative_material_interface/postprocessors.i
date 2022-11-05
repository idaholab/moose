#
# Test use of postprocessor values in parsed materials
#

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Materials]
  [pp]
    type = ParsedMaterial
    expression = 'time^2'
    postprocessor_names = time
    outputs = exodus
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    # make sure the PostProcessor is executed early and often enough
    # when used in the ParsedMaterial (this might have to be on every NONLINEAR
    # or even LINEAR iteration!)
    execute_on = TIMESTEP_BEGIN
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]

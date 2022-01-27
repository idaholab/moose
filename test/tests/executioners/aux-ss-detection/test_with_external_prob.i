[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Problem]
  type = BasicExternalProblem
[]

[Executioner]
  type = Transient
  num_steps = 3
  steady_state_detection = true
  steady_state_tolerance = 1e-2
  check_aux = true
[]

[Outputs]
  exodus = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Convergence]
  [steady_conv]
    type = IterationCountConvergence
    max_iterations = 3
  []
[]

[Executioner]
  type = Transient
  dt = 10
  num_steps = 10
  steady_state_detection = true
  steady_state_convergence = steady_conv
[]


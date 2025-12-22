!include equal_area_with_junction.i

[Components]
  [junction]
    volume = 1.0
  []
[]

[Convergence]
  [components_conv]
    type = ComponentsConvergence
    max_iterations = 10
  []
[]

[Executioner]
  nonlinear_convergence := components_conv
  verbose = true
[]

# Steady-state Cahn-Hilliard (finding equilibrium configuration)
# Energy: F[c] = ∫ [(c^2-1)^2 + κ/2|∇c|^2] dx
# This finds the configuration that minimizes the energy

[Mesh]
  type = GeneratedMesh
  dim = 1  # Start with 1D for simplicity
  nx = 50
  xmin = -1
  xmax = 1
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = FunctionIC
      function = 'tanh(5*x)'  # Initial tanh profile
    []
  []
[]

[AutomaticWeakForm]
  [cahn_hilliard]
    energy_type = expression
    # Double-well with small gradient penalty
    energy_expression = '(c*c - 1.0)*(c*c - 1.0) + 0.5*0.01*dot(grad(c), grad(c))'
    variables = 'c'
    use_automatic_differentiation = true
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = c
    boundary = left
    value = -1
  []
  [right]
    type = DirichletBC
    variable = c
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  
  l_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  nl_max_its = 50
  
  automatic_scaling = true
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
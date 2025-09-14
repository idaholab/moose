# Transient Cahn-Hilliard with automatic differentiation
# Energy: F[c] = ∫ [(c^2-1)^2 + κ/2|∇c|^2] dx
# Evolution: ∂c/∂t = -M * δF/δc = -M * [4c(c^2-1) - κ∇²c]

[Mesh]
  type = GeneratedMesh
  dim = 1
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
      function = '0.1*sin(3.14159*x) + 0.01*sin(10*3.14159*x)'  # Small perturbation
    []
  []
[]

[AutomaticWeakForm]
    energy_type = expression
    energy_expression = '(c*c - 1.0)*(c*c - 1.0) + 0.5*0.01*dot(grad(c), grad(c))'
    variables = 'c'
    use_automatic_differentiation = true
[]

[BCs]
  [left]
    type = NeumannBC
    variable = c
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = c
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  
  # Time stepping
  dt = 0.001
  end_time = 0.1
  
  # Solver settings
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  
  # Tolerances
  l_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  nl_max_its = 20
  
  automatic_scaling = true
[]

[Outputs]
  exodus = true
  perf_graph = true
  interval = 10
[]
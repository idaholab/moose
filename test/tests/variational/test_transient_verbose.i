# Test transient problem with verbose output to check time derivative kernels

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0
  []
[]

[AutomaticWeakForm]
  [heat]
    energy_type = expression
    energy_expression = '0.5*dot(grad(u), grad(u))'
    variables = 'u'
    use_automatic_differentiation = true
    
    # Enable verbose output to see time derivative kernel addition
    verbose = true
    debug_print_weak_form = true
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient  # This should trigger time derivative kernel addition
  dt = 0.1
  end_time = 0.5
  
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = false
  console = true
[]
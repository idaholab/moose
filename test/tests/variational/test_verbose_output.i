# Test verbose output for automatic weak form generation
# This example shows all the debugging and verbose options

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
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
  [diffusion]
    # Basic setup
    energy_type = expression
    energy_expression = '0.5*dot(grad(u), grad(u)) - f*u'
    variables = 'u'
    parameters = 'f 1.0'
    
    # Enable automatic differentiation
    use_automatic_differentiation = true
    
    # Enable all verbose and debug output
    verbose = true                        # Show all processing steps
    debug_print_expressions = true        # Print parsed expressions
    debug_print_derivatives = true        # Show derivative calculations
    debug_print_simplification = true     # Show simplification steps
    debug_print_weak_form = true         # Print final weak form
    debug_print_jacobian = true          # Print Jacobian expressions
    
    # Output weak form to file
    output_weak_form = true
    weak_form_file = 'diffusion_weak_form.txt'
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
  type = Steady
  solve_type = NEWTON
  
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
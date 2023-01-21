[Mesh]
  second_order = true
  [file]
    type = FileMeshGenerator
    file = hex_mesh.e
  []
  [secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 11
    new_block_name = "secondary"
    sidesets = '101'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 12
    new_block_name = "primary"
    sidesets = '102'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [T]
    block = '1 2'
    order = SECOND
  []
  [lambda]
    block = 'secondary'
    # family = MONOMIAL
    # order = CONSTANT
    family = LAGRANGE
    order = SECOND
    use_dual = true
  []
[]

[BCs]
  [neumann]
    type = FunctionGradientNeumannBC
    exact_solution = exact_soln_primal
    variable = T
    boundary = '1 2'
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  []
  [sink]
    type = Reaction
    variable = T
    block = '1 2'
  []
  [forcing_function]
    type = BodyForce
    variable = T
    function = forcing_function
    block = '1 2'
  []
[]

[Functions]
  [forcing_function]
    type = ParsedFunction
    expression = 'sin(x*pi)*sin(y*pi)*sin(z*pi) + 3*pi^2*sin(x*pi)*sin(y*pi)*sin(z*pi)'
  []
  [exact_soln_primal]
    type = ParsedFunction
    expression = 'sin(x*pi)*sin(y*pi)*sin(z*pi)'
  []
  [exact_soln_lambda]
    type = ParsedFunction
    expression = 'pi*sin(pi*y)*sin(pi*z)*cos(pi*x)'
  []
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [mortar]
    type = EqualValueConstraint
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = '12'
    secondary_subdomain = '11'
    variable = lambda
    secondary_variable = T
    # delta = 0.1
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
  petsc_options_value = 'lu       basic                 NONZERO               1e-15'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [L2lambda]
    type = ElementL2Error
    variable = lambda
    function = exact_soln_lambda
    execute_on = 'timestep_end'
    block = 'secondary'
  []
  [L2u]
    type = ElementL2Error
    variable = T
    function = exact_soln_primal
    execute_on = 'timestep_end'
    block = '1 2'
  []
  [h]
    type = AverageElementSize
    block = '1 2'
  []
[]

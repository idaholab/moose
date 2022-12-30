[Mesh]
  second_order = false
  [file]
    type = FileMeshGenerator
    file = spheres_hex8.e
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
  uniform_refine = 0
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [T]
    block = '1 2'
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
    expression= 'x^2 + y^2 + z^2 - 6'
  []
  [exact_soln_primal]
    type = ParsedFunction
    expression= 'x^2 + y^2 + z^2'
  []
  [exact_soln_lambda]
    type = ParsedFunction
    expression= '4'
  []
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [mortar]
    type = PenaltyEqualValueConstraint
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = '12'
    secondary_subdomain = '11'
    secondary_variable = T
    correct_edge_dropping = true
    penalty_value = 1.e5
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
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      6'
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
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

[Mesh]
  [original_file_mesh]
    type = FileMeshGenerator
    file = non_conform_2blocks.e
  []
  [secondary_side]
    input = original_file_mesh
    type = LowerDBlockFromSidesetGenerator
    sidesets = '10'
    new_block_id = '100'
    new_block_name = 'secondary_side'
  []
  [primary_side]
    input = secondary_side
    type = LowerDBlockFromSidesetGenerator
    sidesets = '20'
    new_block_id = '200'
    new_block_name = 'primary_side'
  []
[]

[Functions]
  [exact_sln]
    type = ParsedFunction
    expression = sin(2*pi*x)*sin(2*pi*y)
  []
  [ffn]
    type = ParsedFunction
    expression = 8*pi*pi*sin(2*pi*x)*sin(2*pi*y)
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  []

  [lm]
    order = FIRST
    family = LAGRANGE
    block = secondary_side
    use_dual = true
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [ffn]
    type = BodyForce
    variable = u
    function = ffn
  []
[]

[Constraints]
  [ced]
    type = EqualValueConstraint
    variable = lm
    secondary_variable = u
    primary_boundary = 20
    primary_subdomain = 200
    secondary_boundary = 10
    secondary_subdomain = 100
  []
[]

[BCs]
  [all]
    type = DirichletBC
    variable = u
    boundary = '30 40'
    value = 0.0
  []
  [neumann]
    type = FunctionGradientNeumannBC
    exact_solution = exact_sln
    variable = u
    boundary = '50 60'
  []
[]

[Postprocessors]
  [l2_error]
    type = ElementL2Error
    variable = u
    function = exact_sln
    block = '1 2'
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [vcp]
    type = VCP
    full = true
    lm_variable = 'lm'
    primary_variable = 'u'
    preconditioner = 'AMG'
    is_lm_coupling_diagonal = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_view'

  l_max_its = 100
  nl_rel_tol = 1e-6
[]

[Outputs]
  csv = true
[]

[Mesh]
  second_order = true
  [./file]
    type = FileMeshGenerator
    file = spheres_hex8.e
  [../]
  [./secondary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 12
    new_block_name = "secondary"
    sidesets = '102'
  [../]
  [./primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 11
    new_block_name = "primary"
    sidesets = '101'
  [../]
  [./outer]
    input = primary
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 1002
    new_block_name = "outer"
    sidesets = '2'
  [../]
  [./inner]
    input = outer
    type = LowerDBlockFromSidesetGenerator
    new_block_id = 1001
    new_block_name = "inner"
    sidesets = '1'
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./T]
    block = '1 2'
    order = FIRST
  [../]
  [./lambda]
    block = 'secondary'
    # family = MONOMIAL
    # order = CONSTANT
    family = LAGRANGE
    order = FIRST
    # use_dual = true
  [../]
[]

[BCs]
  [./neumann]
    type = FunctionGradientNeumannBC
    exact_solution = exact_soln_primal
    variable = T
    boundary = '1 2'
  [../]
[]

[Kernels]
  [./conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  [../]
  [./sink]
    type = Reaction
    variable = T
    block = '1 2'
  [../]
  [./forcing_function]
    type = BodyForce
    variable = T
    function = forcing_function
    block = '1 2'
  [../]
[]

[Functions]
 [./forcing_function]
 type = ParsedFunction
 value = 'x^2 + y^2 + z^2-6'
 [../]
 [./exact_soln_primal]
 type = ParsedFunction
 value = 'x^2 + y^2 + z^2'
 [../]
 [exact_soln_lambda]
 type = ParsedFunction
 value = '-4'
 []
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [./mortar]
    type = EqualValueConstraint
    primary_boundary = 1
    secondary_boundary = 2
    primary_subdomain = '11'
    secondary_subdomain = '12'
    variable = lambda
    secondary_variable = T
    delta = .1
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       basic                 NONZERO               1e-15'
[]

[Outputs]
 [dofmap]
   type = DOFMap
   execute_on = 'initial'
 []
 [oversample]
    type = Exodus
    refinements = 1
 []
 exodus = true
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

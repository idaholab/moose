[Mesh]
  second_order = true
  [file]
    type = FileMeshGenerator
    file = nodal_normals_test_offset_nonmatching_gap.e
  []
  [./primary]
    input = file
    type = LowerDBlockFromSidesetGenerator
    sidesets = '2'
    new_block_id = '20'
  [../]
  [./secondary]
    input = primary
    type = LowerDBlockFromSidesetGenerator
    sidesets = '1'
    new_block_id = '10'
  [../]
[]

[Variables]
  [./T]
    block = '1 2'
    order = SECOND
  [../]
  [./lambda]
    block = '10'
  [../]
[]

[AuxVariables]
  [ssm]
    order = CONSTANT
    family = MONOMIAL
    block = '1 2'
  []
[]

[BCs]
  [./neumann]
    type = FunctionGradientNeumannBC
    exact_solution = exact_soln
    variable = T
    boundary = '3 4 5 6 7 8'
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

[AuxKernels]
  [ssm]
    type = MaterialRealAux
    variable = ssm
    property = diffusivity
    block = '1 2'
  []
[]

[Materials]
  [./ssm]
    type = SpatialStatefulMaterial
    block = '1 2'
  [../]
[]

[Functions]
  [./forcing_function]
    type = ParsedFunction
    expression= '-4 + x^2 + y^2'
  [../]
  [./exact_soln]
    type = ParsedFunction
    expression= 'x^2 + y^2'
  [../]
[]

[Debug]
  show_var_residual_norms = 1
[]

# This is the consumer of material properties, through its default diffusivity parameters
[Constraints]
  [./mortar]
    type = EqualValueConstraint
    primary_boundary = 2
    secondary_boundary = 1
    primary_subdomain = 20
    secondary_subdomain = 10
    variable = lambda
    secondary_variable = T
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
  nl_abs_tol = 1e-12
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       basic                 NONZERO               1e-15'
  num_grids = 2
[]

[Outputs]
  exodus = true
[]

[Adaptivity]
  steps = 1
  marker = uniform
  [Markers]
    [uniform]
      type = UniformMarker
      mark = refine
    []
  []
[]

[Mesh]
  inactive = 'translate'
  [file]
    type = FileMeshGenerator
    file = 2blk-gap.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '101'
    new_block_id = 10001
    new_block_name = 'secondary_lower'
    input = file
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '100'
    new_block_id = 10000
    new_block_name = 'primary_lower'
    input = secondary
  []
  [translate]
    type = TransformGenerator
    transform = translate
    input = primary
    vector_value = '1 0 0'
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Variables]
  [temp]
    type = MooseVariableFVReal
    block = '1 2'
  []
  [lm]
    order = CONSTANT
    family = MONOMIAL
    block = 'secondary_lower'
  []
[]

[Materials]
  [left]
    type = ADGenericFunctorMaterial
    block = 1
    prop_names = 'thermal_conductivity'
    prop_values = '0.01'
  []

  [right]
    type = ADGenericFunctorMaterial
    block = 2
    prop_names = 'thermal_conductivity'
    prop_values = '0.005'
  []
[]

[FVKernels]
  [hc]
    type = FVDiffusion
    variable = temp
    block = '1 2'
    coeff = 'thermal_conductivity'
  []
[]

[UserObjects]
  [radiation]
    type = FunctorGapFluxModelRadiation
    temperature = temp
    boundary = 100
    primary_emissivity = 1.0
    secondary_emissivity = 1.0
  []
  [conduction]
    type = FunctorGapFluxModelConduction
    temperature = temp
    boundary = 100
    gap_conductivity = 0.02
  []
[]

[Constraints]
  [ced]
    type = ModularGapConductanceConstraint
    variable = lm
    secondary_variable = temp
    primary_boundary = 100
    primary_subdomain = 10000
    secondary_boundary = 101
    secondary_subdomain = 10001
    gap_flux_models = 'radiation conduction'
    ghost_higher_d_neighbors = true
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = temp
    boundary = 'left'
    value = 100
  []

  [right]
    type = FVDirichletBC
    variable = temp
    boundary = 'right'
    value = 0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-11
  nl_abs_tol = 1.0e-10
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]

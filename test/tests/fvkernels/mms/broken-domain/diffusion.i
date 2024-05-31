[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface_primary_side]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary_interface'
  []
  [interface_secondary_side]
    input = interface_primary_side
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'secondary_interface'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    block = 0
    initial_condition = 0.5
  []
  [v]
    type = MooseVariableFVReal
    block = 1
    initial_condition = 0.5
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = u
    coeff = 'left'
    block = 0
  []
  [diff_right]
    type = FVDiffusion
    variable = v
    coeff = 'right'
    block = 1
  []
  [body_left]
    type = FVBodyForce
    variable = u
    function = 'forcing'
    block = 0
  []
  [body_right]
    type = FVBodyForce
    variable = v
    function = 'forcing'
    block = 1
  []
[]

[FVInterfaceKernels]
  # This will add a flux term for variable1, e.g. u
  [interface]
    type = FVOnlyAddDiffusionToOneSideOfInterface
    variable1 = u
    variable2 = v
    boundary = 'primary_interface'
    subdomain1 = '0'
    subdomain2 = '1'
    coeff2 = 'right'
  []
[]

[FVBCs]
  [left]
    type = FVFunctionDirichletBC
    variable = u
    boundary = 'left'
    function = 'exact'
  []
  [right]
    type = FVFunctionDirichletBC
    variable = v
    boundary = 'right'
    function = 'exact'
  []
  [middle]
    # by adding a dirichlet BC we ensure that flux kernels will run for variable v
    type = FVADUseFunctorSideForSsfDirichletBC
    variable = v
    functor = u
    boundary = 'secondary_interface'
  []
[]

[FunctorMaterials]
  [block0]
    type = ADGenericFunctorMaterial
    block = '0'
    prop_names = 'left'
    prop_values = '1'
  []
  [block1]
    type = ADGenericFunctorMaterial
    block = '1'
    prop_names = 'right'
    prop_values = '1'
  []
  [composite]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'composite'
    subdomain_to_prop_value = '0 u 1 v'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm lu NONZERO'
[]

[Outputs]
  exodus = true
  csv = true
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = '3*x^2 + 2*x + 1'
  []
  [forcing]
    type = ParsedFunction
    expression = '-6'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2FunctorError
    approximate = composite
    exact = exact
    outputs = 'console csv'
  []
  [h]
    type = AverageElementSize
    outputs = 'console csv'
  []
[]

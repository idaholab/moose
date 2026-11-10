[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[Variables]
  [u]
  []
  [s]
    family = SCALAR
    initial_condition = 3
  []
[]

[FunctorMaterials]
  # Defined on all blocks and forwarding to a scalar variable, which does not need mesh information
  [scalar_prop]
    type = ADGenericFunctorMaterial
    prop_names = scalar_prop
    prop_values = s
  []
  # Defined on all blocks and forwarding to a field variable, which needs mesh information
  [field_prop]
    type = ADGenericFunctorMaterial
    prop_names = field_prop
    prop_values = u
  []
  # Defined on only one of the two blocks
  [block_prop]
    type = ADGenericFunctorMaterial
    prop_names = block_prop
    prop_values = 2
    block = 1
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

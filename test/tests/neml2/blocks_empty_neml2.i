# This example tests the case where one rank doesn't have any NEML2 constitutive update
#
# Specifically, a block partitioner is used to separate the domains. Rank 0 gets block A, and Rank 1 gets block B.
# NEML2 materials are only defined for block A, hence rank 1 will not have any NEML2 updates.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [A]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    block_name = A
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [B]
    type = SubdomainBoundingBoxGenerator
    input = A
    block_id = 2
    block_name = B
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
  [Partitioner]
    type = GridPartitioner
    nx = 2
    ny = 1
    nz = 1
  []
[]

[NEML2]
  eager = true
  input = 'models/custom_model.i'
  load = 'models/test_models.py'
  verbose = true
  device = 'cpu'
  [A]
    model = 'model_A'
    block = 'A'

    # request derivatives (must be pairs of two)
    # derivative name follow moose convention, e.g., 'doutput/dinput'
    derivatives = 'product A'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [reaction]
    type = Reaction
    variable = u
  []
  [source_1]
    type = MatBodyForce
    variable = u
    material_property = sum
  []
  [source_2]
    type = MatBodyForce
    variable = u
    material_property = product
  []
  [source_3]
    type = MatBodyForce
    variable = u
    material_property = dproduct/dA
  []
[]

[AuxVariables]
  [A]
  []
  [pid]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = ProcessorIDAux
      execute_on = 'INITIAL'
    []
  []
[]

[ICs]
  [A]
    type = FunctionIC
    variable = A
    function = 'x'
  []
[]

[Materials]
  [B]
    type = GenericFunctionMaterial
    prop_names = 'B'
    prop_values = 'y+t'
  []
  [dummy]
    type = GenericConstantMaterial
    prop_names = 'sum product dproduct/dA'
    prop_values = '0 0 0'
    block = 'B'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 2
[]

[Outputs]
  exodus = true
[]

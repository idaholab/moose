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
  input = 'models/custom_model.i'
  verbose = true
  device = 'cpu'
  [A]
    model = 'model_A'
    block = 'A'

    moose_input_types = 'VARIABLE MATERIAL'
    moose_inputs = '     a        b'
    neml2_inputs = '     forces/A forces/B'

    moose_output_types = 'MATERIAL           MATERIAL'
    moose_outputs = '     neml2_sum          neml2_product'
    neml2_outputs = '     state/internal/sum state/internal/product'

    moose_derivative_types = 'MATERIAL'
    moose_derivatives = 'neml2_dproduct_da'
    neml2_derivatives = 'state/internal/product forces/A'
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
    material_property = neml2_sum
  []
  [source_2]
    type = MatBodyForce
    variable = u
    material_property = neml2_product
  []
  [source_3]
    type = MatBodyForce
    variable = u
    material_property = neml2_dproduct_da
  []
[]

[AuxVariables]
  [a]
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
  [a]
    type = FunctionIC
    variable = a
    function = 'x'
  []
[]

[Materials]
  [b]
    type = GenericFunctionMaterial
    prop_names = 'b'
    prop_values = 'y+t'
  []
  [dummy]
    type = GenericConstantMaterial
    prop_names = 'neml2_sum neml2_product neml2_dproduct_da'
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

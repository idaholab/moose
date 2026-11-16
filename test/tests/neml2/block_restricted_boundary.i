[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = QUAD9
    nx = 4
    ny = 3
  []
  [left_block]
    type = SubdomainBoundingBoxGenerator
    input = generated
    block_id = 0
    block_name = left_block
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [right_block]
    type = SubdomainBoundingBoxGenerator
    input = left_block
    block_id = 1
    block_name = right_block
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = right_block
    new_boundary = left_block_right_block
    primary_block = left_block
    paired_block = right_block
  []
[]

[NEML2]
  eager = true
  input = 'models/custom_model.i'
  load = 'models/test_models.py'
  inputs = 'A B'
  input_types = 'VARIABLE MATERIAL'
  [domains]
    model = model
    device = cpu
    block = left_block
    volume = true
    boundary = 'left left_block_right_block'
  []
[]

[Problem]
  kernel_coverage_check = false
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
[]

[AuxVariables]
  [A]
  []
  [neml2_volume]
    family = MONOMIAL
    order = CONSTANT
    block = left_block
  []
  [neml2_boundary]
    family = SIDE_HIERARCHIC
    order = CONSTANT
  []
[]

[ICs]
  [A]
    type = FunctionIC
    variable = A
    function = x
  []
[]

[Materials]
  [B_left]
    type = GenericFunctionMaterial
    block = left_block
    prop_names = B
    prop_values = '10+y+t'
  []
  [B_right]
    type = GenericFunctionMaterial
    block = right_block
    prop_names = B
    prop_values = '100+y+t'
  []
[]

[AuxKernels]
  [volume]
    type = MaterialRealAux
    variable = neml2_volume
    property = sum
    block = left_block
    execute_on = TIMESTEP_END
  []
  [boundary]
    type = MaterialRealAux
    variable = neml2_boundary
    property = sum
    boundary = 'left left_block_right_block'
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]

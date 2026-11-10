[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
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
  [mixed_topology]
    type = SideSetsAroundSubdomainGenerator
    input = interface
    block = left_block
    new_boundary = mixed_topology
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
    volume = true
    boundary = left
    interface = left_block_right_block
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
[]

[ICs]
  [A]
    type = FunctionIC
    variable = A
    function = x
  []
[]

[Materials]
  [B]
    type = GenericFunctionMaterial
    prop_names = B
    prop_values = 'y+t'
  []
[]

[Postprocessors]
  [volume]
    type = ElementAverageMaterialProperty
    mat_prop = sum
    execute_on = TIMESTEP_END
  []
  [boundary]
    type = SideAverageMaterialProperty
    property = sum
    boundary = left
    execute_on = TIMESTEP_END
  []
  [interface]
    type = SideAverageMaterialProperty
    property = sum
    boundary = left_block_right_block
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]

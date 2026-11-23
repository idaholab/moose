# The same boundary pair carries one mortar interface on the reference mesh and one on the displaced
# mesh. On a distributed mesh with more processes than elements, some processes own no mortar
# segments once the mesh is distributed, so they see the mortar coverage change when the others do
# not.
[Mesh]
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    boundary_name_prefix = left
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    boundary_name_prefix = right
    boundary_id_offset = 10
    subdomain_ids = 1
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'left right'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = combined
    sidesets = left_right
    new_block_id = 10
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = right_left
    new_block_id = 20
  []
  displacements = 'disp_x disp_y'
  parallel_type = distributed
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Variables]
  [u]
    block = '0 1'
  []
  [lm_reference]
    block = 10
  []
  [lm_displaced]
    block = 10
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = '0 1'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left_left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right_right
    value = 1
  []
[]

[Constraints]
  [reference]
    type = EqualValueConstraint
    primary_boundary = right_left
    secondary_boundary = left_right
    primary_subdomain = 20
    secondary_subdomain = 10
    variable = lm_reference
    secondary_variable = u
  []
  [displaced]
    type = EqualValueConstraint
    primary_boundary = right_left
    secondary_boundary = left_right
    primary_subdomain = 20
    secondary_subdomain = 10
    variable = lm_displaced
    secondary_variable = u
    use_displaced_mesh = true
  []
[]

[Postprocessors]
  [u_average]
    type = ElementAverageValue
    variable = u
    block = '0 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]

# This input file exports volumetric and boundary stateful material property data from a
# two-block mesh. The side averages on the exterior boundary "right" and on the internal sideset
# "interface" between the blocks make the face copies of the stateful material compute (and
# store stateful data) on those sides.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
  [right_block]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = right_block
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[Variables]
  [u]
    initial_condition = 0
  []
[]

[Kernels]
  [diff]
    type = MatDiffusionTest
    variable = u
    prop_name = diffusivity
    prop_state = 'old'
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [stateful]
    type = SpatialStatefulMaterial
    initial_diffusivity = 1.0
  []
[]

[Postprocessors]
  [right_diffusivity]
    type = SideAverageMaterialProperty
    property = diffusivity
    boundary = right
  []
  [interface_diffusivity]
    type = SideAverageMaterialProperty
    property = diffusivity
    boundary = interface
  []
[]

[UserObjects]
  [exporter]
    type = StatefulMaterialPropertyExporter
    file_base = 'boundary_export'
    execute_on = FINAL
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 3
  dt = 0.1
[]

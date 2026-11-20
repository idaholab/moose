# This input file imports the volumetric and boundary stateful material property data exported
# by export_boundary.i onto a finer mesh. ImportCheckStatefulMaterial errors at the first
# timestep at any volume or face quadrature point whose diffusivity_old was not overwritten by
# the import, which covers the face copies on "right" and "interface".

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
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
    type = ImportCheckStatefulMaterial
    initial_diffusivity = 10.0
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
  [importer]
    type = StatefulMaterialPropertyImporter
    file_base = 'boundary_export'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 1
  dt = 0.1
[]

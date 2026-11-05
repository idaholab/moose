# This input file exports stateful material property data from a mesh where only one of two
# subdomains has volume materials. The other subdomain only has a boundary material on one of
# its sidesets, which makes its elements pass through the volumetric stateful storage without
# ever being initialized with stateful data, so the exporter must skip them.

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
[]

[Problem]
  # Block 1 intentionally has no volume material
  material_coverage_check = false
[]

[Variables]
  [u]
    initial_condition = 0
  []
[]

[Kernels]
  [diff_stateful]
    type = MatDiffusionTest
    variable = u
    prop_name = diffusivity
    block = 0
  []
  [diff_plain]
    type = Diffusion
    variable = u
    block = 1
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
    type = MatNeumannBC
    variable = u
    boundary = right
    value = 1
    boundary_material = flux_coef
  []
[]

[Materials]
  [stateful]
    type = SpatialStatefulMaterial
    initial_diffusivity = 1.0
    block = 0
  []
  [boundary_only]
    type = GenericConstantMaterial
    prop_names = flux_coef
    prop_values = 2.0
    boundary = right
  []
[]

[UserObjects]
  [exporter]
    type = StatefulMaterialPropertyExporter
    file_base = 'mixed_blocks_export'
    execute_on = FINAL
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 3
  dt = 0.1
[]

# This input file imports from export_subset_source.i onto a target simulation that only
# declares the "diffusivity" stateful property. The importer should skip the exported
# "conductivity" history with a warning instead of erroring out.

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

[UserObjects]
  [importer]
    type = StatefulMaterialPropertyImporter
    file_base = 'subset_export'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 1
  dt = 0.1
[]

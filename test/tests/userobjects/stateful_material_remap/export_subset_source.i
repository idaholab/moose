# This input file exports two stateful properties ("diffusivity" and "conductivity")
# so import_subset.i can verify that the importer skips exported properties that are
# not declared in the current simulation.

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
    type = PartialStatefulMaterial
    initial_diffusivity = 1.0
    initial_conductivity = 2.0
  []
[]

[UserObjects]
  [exporter]
    type = StatefulMaterialPropertyExporter
    file_base = 'subset_export'
    execute_on = FINAL
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 3
  dt = 0.1
[]

# Tests partial import: the material has two stateful properties ("diffusivity" and
# "conductivity"), but only "diffusivity" is present in the import file.
# After import, PartialStatefulMaterial::computeQpProperties() asserts at the first
# timestep that:
#   - diffusivity_old was overwritten by the importer (remapped value, not initial_diffusivity)
#   - conductivity_old equals initial_conductivity, proving initStatefulProperties()
#     ran correctly for the non-imported property (not zero-initialized)

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
    type = PartialStatefulMaterial
    initial_diffusivity = 1.0
    initial_conductivity = 2.0
  []
[]

[UserObjects]
  [importer]
    type = StatefulMaterialPropertyImporter
    file_base = 'stateful_export'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 1
  dt = 0.1
[]

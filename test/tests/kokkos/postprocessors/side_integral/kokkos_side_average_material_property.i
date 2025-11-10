[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
  xmax = 4
  ymax = 1
[]

[Variables]
  [u]
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Materials]
  [real]
    type = KokkosGenericConstantMaterial
    prop_names = prop
    prop_values = 1.1
  []
[]

[Postprocessors]
  [average]
    type = KokkosSideAverageMaterialProperty
    boundary = bottom
    property = prop
  []
[]

[Outputs]
  csv = true
[]

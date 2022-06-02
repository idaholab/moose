#
# Common input for side_integral_material_property.i and
# side_average_material_property.i
#

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
  active = 'types'
  [types]
    type = TypesMaterial
    std_vec_prop_entry1 = 1.1
  []

  # while TypeMaterial supplies the types below as well, we're providing custom
  # ones to ensure that the average stays the same and we can use a single CSV
  # gold file for the test
  [real]
    type = GenericConstantMaterial
    prop_names = prop
    prop_values = 1.1
  []
  [realvector]
    type = GenericConstantVectorMaterial
    prop_names = prop
    prop_values = '2.2 1.1 3.3'
  []
  [ranktwo]
    type = GenericConstantRankTwoTensor
    tensor_name = prop
    tensor_values = '3.3 2.2 1.1 4.4 5.5 6.6'
  []
[]

[Outputs]
  csv = true
[]

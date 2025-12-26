[Mesh]
  [./eg]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "C0POLYGON"
    subdomain_name = 'base'
  []
[]

[Postprocessors]
  [vol]
    type = VolumePostprocessor
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

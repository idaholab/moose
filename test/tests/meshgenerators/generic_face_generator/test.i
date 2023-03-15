[Mesh]
  [face]
    type = GenericFaceGenerator
    nodal_positions = "1 0 0
                       0 1 0
                       0.5 0 0
                       0 0.5 0
                       0.5 0.5 0
                       1 0.5 0
                       0.5 1 0"

    element_connectivity = '0 1 2; 0 2 3 5; 2 3 4 5 6; 0 1 2 3 4 5 6'
  []
[]

[Variables]
  [u]
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

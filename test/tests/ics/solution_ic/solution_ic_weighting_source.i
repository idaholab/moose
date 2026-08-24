[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 1
  []
[]

# Use the element ID itself as the source value so the expected weighting results
# are independent of element ordering when a distributed mesh is serialized to Exodus.
[AuxVariables]
  [source_value]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_source_value]
    type = ElementIDAux
    variable = source_value
    execute_on = INITIAL
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = INITIAL
  []
[]

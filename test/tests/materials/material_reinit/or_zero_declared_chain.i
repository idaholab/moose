# A material retrieves with getGenericOrZeroMaterialProperty a property that another material
# declares, so it must read the declared value rather than zero.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[AuxVariables]
  [consumer]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [consumer]
    type = MaterialRealAux
    variable = consumer
    property = consumer
  []
[]

[Materials]
  # consumer = 1 + A = 3 when A is computed, 1 if A were read as zero
  [consumer]
    type = MaterialReinitTest
    property = consumer
    value = 1
    coupled_property = A
    or_zero_coupled_property = true
  []
  # declared after the consumer, so it is not constructed yet when the consumer retrieves A
  [A]
    type = MaterialReinitTest
    property = A
    value = 2
  []
[]

[Postprocessors]
  [consumer]
    type = ElementAverageValue
    variable = consumer
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

[Mesh]
  type = AbaqusUELMesh
  file = CUBE_UEL.inp
  debug = true

  [Partitioner]
    type = LibmeshPartitioner
    partitioner = hilbert_sfc
  []
[]

[Variables]

  [AddUELVariables]
  []
[]

# [ICs/AddUELICs]
# []

[AuxVariables]
  [pid]
  []
[]

[AuxKernels]
  [pid]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[UserObjects]
  [uel]
    type = AbaqusUELMeshUserElement
    uel_type = U1
    plugin = elasticity_uel/uel
    element_sets = CUBE
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Mesh]
  type = AbaqusUELMesh
  file = BIG_CUBE_UEL.inp
  debug = true

  [Partitioner]
    type = LibmeshPartitioner
    partitioner = hilbert_sfc
  []
[]

[Variables/AddUELVariables]
[]

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

[ICs]
  [var_1]
    type = ConstantIC
    value = 1
    variable = var_1
  []
  [var_2]
    type = ConstantIC
    value = 2
    variable = var_2
  []
  [var_4]
    type = ConstantIC
    value = 4
    variable = var_4
  []
  [var_8]
    type = ConstantIC
    value = 8
    variable = var_8
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[UserObjects]
  [uel]
    type = AbaqusUELMeshUserElement
    uel_type = U1
    plugin = elasticity_uel/uel
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

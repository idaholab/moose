[Mesh]
  type = AbaqusUELMesh
  file = ELASTIC_PATCH.inp
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

# those will be added by the action
[BCs]
  [disp_x]
    type = AbaqusEssentialBC
    abaqus_var_id = 1
    variable = disp_x
    abaqus_step = Step-1
  []
  [disp_y]
    type = AbaqusEssentialBC
    abaqus_var_id = 2
    variable = disp_y
    abaqus_step = Step-1
  []
[]

[UserObjects]
  [ics]
    type = AbaqusUELInitialCondition
  []
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

[Problem]
  kernel_coverage_check = false
[]

[UserObjects]
  [uel]
    type = AbaqusUELMeshUserElement
    uel_type = U1
    plugin = ../../plugins/small_strain_tri_uel
    element_sets = CUBE
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

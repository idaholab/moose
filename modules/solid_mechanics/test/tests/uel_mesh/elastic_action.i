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
  extra_tag_vectors = "AbaqusUELTag"
[]

# the action adds the AbaqusEssentialBC, AbaqusForceBC, and AbaqusUELStepUserObject objects
[BCs]
  [Abaqus]
  []
[]

[UserObjects]
  [step_uo]
    type = AbaqusUELStepUserObject
  []
  [uel]
    type = AbaqusUELMeshUserElement
    uel_type = U1
    plugin = ../../plugins/small_strain_tri_uel
    element_sets = CUBE
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  dtmin = 0.1
  end_time = 2
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]

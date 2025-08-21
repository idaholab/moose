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

# those will be added by the action
[BCs]
  [disp_x]
    type = AbaqusEssentialBC
    abaqus_var_id = 1
    variable = disp_x
    step_user_object = step_uo
  []
  [disp_y]
    type = AbaqusEssentialBC
    abaqus_var_id = 2
    variable = disp_y
    step_user_object = step_uo
  []
[]

[NodalKernels]
  [disp_x]
    type = AbaqusForceBC
    abaqus_var_id = 1
    variable = disp_x
    step_user_object = step_uo
  []
  [disp_y]
    type = AbaqusForceBC
    abaqus_var_id = 2
    variable = disp_y
    step_user_object = step_uo
  []
[]

[UserObjects]
  [step_uo]
    type = AbaqusUELStepUserObject
  []
  [dload_uo]
    type = AbaqusDLoadInterpolator
    step_user_object = step_uo
  []
  [uel]
    type = AbaqusUELMeshUserElement
    uel_type = U1
    plugin = ../../plugins/small_strain_tri_uel
    element_sets = CUBE
    dload_interpolator = dload_uo
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

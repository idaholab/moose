[Mesh]
  type = AbaqusUELMesh
  file = CUBE_UEL_BCS.inp
  debug = true

  [Partitioner]
    type = LibmeshPartitioner
    partitioner = hilbert_sfc
  []
[]

[Problem]
  extra_tag_vectors = "AbaqusUELTag"
[]

[Variables]
  [AddUELVariables]
  []
[]

[AuxVariables]
  [field1]
  []
  [field2]
  []
[]

[AuxKernels]
  [field1]
    type = AbaqusPredefAux
    field = 1
    variable = field1
  []
  [field2]
    type = AbaqusPredefAux
    field = 2
    variable = field2
  []
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

[Problem]
  kernel_coverage_check = false
[]

[UserObjects]
  [uel]
    type = AbaqusUELMeshUserElement
    uel_type = U1
    plugin = elasticity_uel/uel
    element_sets = CUBE
    external_fields = "field1 field2"
  []
  [step_uo]
    type = AbaqusUELStepUserObject
  []
[]

[VectorPostprocessors]
  [statev]
    type = AbaqusUELStateVariables
    uel = uel
    split = 9
    column_names = 'not_used SSE S11 S22 S33 S12 E11 E22 E33 E12'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = NONE
  dt = 0.1
  dtmin = 0.1
  end_time = 3
  nl_abs_tol = 1e-9
  # petsc_options_iname = '-pc_factor_shift_type'
  # petsc_options_value = 'nonzero'
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]

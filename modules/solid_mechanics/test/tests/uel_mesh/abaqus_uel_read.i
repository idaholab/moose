[Mesh]
  type = AbaqusUELMesh
  file = PATCH_UEL.inp
  # file = CUBE_UEL.inp
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
    abaqus_step = Step-1
  []
  [disp_y]
    type = AbaqusEssentialBC
    abaqus_var_id = 2
    variable = disp_y
    abaqus_step = Step-1
  []
  [rot_x]
    type = AbaqusEssentialBC
    abaqus_var_id = 4
    variable = rot_x
    abaqus_step = Step-1
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
    plugin = elasticity_uel/uel
    element_sets = CUBE
    external_fields = "field1 field2"
  []
[]

[Preconditioning]
  [fdp]
    type = FDP
    full = true
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
  end_time = 1.0
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]

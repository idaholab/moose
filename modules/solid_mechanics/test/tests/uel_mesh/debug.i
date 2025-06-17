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
    type = FunctionDirichletBC
    boundary = abaqus_bc_union_boundary
    variable = disp_x
    function = 0.1*y
  []
  [disp_y]
    type = FunctionDirichletBC
    boundary = abaqus_bc_union_boundary
    variable = disp_y
    function = 0.1*x
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

[Preconditioning]
  # [fdp]
  #   type = FDP
  #   full = true
  # []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = NONE
  dt = 0.1
  end_time = 2.999999
  nl_abs_tol = 1e-9
  automatic_scaling = true
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # petsc_options_iname = '-pc_factor_shift_type'
  # petsc_options_value = 'nonzero'
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]

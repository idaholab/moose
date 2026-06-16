[Mesh]
  type = AbaqusUELMesh
  file = BIG_CUBE_UEL.inp
  debug = true
[]

[Variables]
  [AddUELVariables]
  []
[]

[AuxVariables]
  [pid]
  []
  [field1]
    initial_condition = 70.0
  []
  [field2]
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [pid]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
  []
[]

# the action adds the AbaqusEssentialBC, AbaqusForceBC, and AbaqusUELStepUserObject objects
[BCs]
  [Abaqus]
  []
[]

[Problem]
  kernel_coverage_check = false
  extra_tag_vectors = "AbaqusUELTag"
[]

[UserObjects]
  [dload_uo]
    type = AbaqusDLoadInterpolator
    step_user_object = abaqus_step_uo
  []
  [uel]
    type = AbaqusUELMeshUserElement
    uel_type = U1
    plugin = elasticity_uel/uel
    element_sets = CUBE
    external_fields = "field1 field2"
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = NONE
  dt = 0.1
  end_time = 1.0
  nl_abs_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  exodus = true
  hide = pid
[]

[Mesh]
  type = AbaqusUELMesh
  file = PATCH_UEL.inp
  debug = true
[]

[Variables]
  [AddUELVariables]
  []
[]

[AuxVariables]
  [field1]
    initial_condition = 70.0
  []
  [field2]
    initial_condition = 0.0
  []
[]

# the action adds the AbaqusEssentialBC, AbaqusForceBC, and AbaqusUELStepUserObject objects
[BCs]
  [Abaqus]
  []
[]

[Problem]
  extra_tag_vectors = "AbaqusUELTag"
  kernel_coverage_check = false
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
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]

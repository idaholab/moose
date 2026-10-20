[Mesh]
  type = AbaqusUELMesh
  file = CUBE_UEL_AMP.inp
[]

[Problem]
  extra_tag_vectors = "AbaqusUELTag"
[]

[Functions]
  [AddUELFunctions]
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
    step_user_object = step_uo
  []
[]

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
    plugin = elasticity_uel/uel
    element_sets = CUBE
    external_fields = "field1 field2"
  []
[]

[Postprocessors]
  [field2_node1]
    type = NodalVariableValue
    variable = field2
    nodeid = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = NONE
  dt = 0.1
  end_time = 2.999999
  nl_abs_tol = 1e-9
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  csv = true
  print_linear_residuals = false
[]

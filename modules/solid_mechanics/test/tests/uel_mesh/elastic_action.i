[Mesh]
  type = AbaqusUELMesh
  file = ELASTIC_PATCH.inp
  debug = true
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
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  exodus = true
  hide = pid
[]

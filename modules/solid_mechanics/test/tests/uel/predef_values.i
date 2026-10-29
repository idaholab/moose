# The field phi = 100 + t is set only at the start of each time step, and the UEL reports the
# PREDEF value and increment it receives, averaged over the nodes of the single element.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = QUAD4
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [phi]
  []
[]

[AuxKernels]
  [phi]
    type = FunctionAux
    variable = phi
    function = '100 + t'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[UserObjects]
  [uel]
    type = AbaqusUserElement
    variables = u
    external_fields = phi
    plugin = ../../plugins/predef_probe_uel
    use_displaced_mesh = false
    num_state_vars = 0
    use_energy = true
    constant_properties = '0'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1
  num_steps = 3
[]

[Postprocessors]
  [predef_value]
    type = AbaqusTotalUELEnergy
    uel_user_object = uel
    component = 0
  []
  [predef_increment]
    type = AbaqusTotalUELEnergy
    uel_user_object = uel
    component = 1
  []
[]

[Outputs]
  csv = true
[]

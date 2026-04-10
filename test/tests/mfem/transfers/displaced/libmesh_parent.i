[Mesh]
  type = FileMesh
  file = ./displaced_indicator_strip.e
  displacements = 'disp_x disp_y'
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  verbose_multiapps = true
[]

[AuxVariables]
  [background_field]
  []
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxKernels]
  [move]
    type = FunctionAux
    variable = disp_x
    function = 't'
    execute_on = 'TIMESTEP_END'
  []
[]

[MultiApps]
  [background]
    type = TransientMultiApp
    input_files = mfem_child.i
    execute_on = 'initial timestep_begin'
  []
[]

[Transfers]
  [pull_indicator_nodal]
    type = MultiAppMFEMTolibMeshShapeEvaluationTransfer
    from_multi_app = background
    source_variable = indicator_field
    variable = background_field
    displaced_target_mesh = true
    execute_on = 'initial timestep_begin'
  []
[]

[Executioner]
  type = Transient
  dt = 0.05
  num_steps = 10
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

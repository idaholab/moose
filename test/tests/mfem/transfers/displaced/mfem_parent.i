[Problem]
  type = MFEMProblem
  solve = false
[]

[Mesh]
  type = MFEMMesh
  file = base_strip.e
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[AuxVariables]
  [indicator_nodal]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Executioner]
  type = MFEMTransient
  dt = 0.05
  num_steps = 10
[]

[MultiApps]
  [solid_domain]
    type = TransientMultiApp
    input_files = libmesh_child.i
    execute_on = 'initial timestep_begin'
  []
[]

[Transfers]
  [pull_indicator_nodal]
    type = MultiApplibMeshToMFEMShapeEvaluationTransfer
    from_multi_app = solid_domain
    source_variables = solid_indicator
    variables = indicator_nodal
    displaced_source_mesh = true
    execute_on = 'initial timestep_begin'
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Displaced
    vtk_format = ASCII
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

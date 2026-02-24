[Mesh]
  type = MFEMMesh
  file = ../../mesh/cylinder-hex-q2.gen
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [potential]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[MultiApps]
  [mfem_app]
    type = FullSolveMultiApp
    input_files = mfem_sub_embedded_submesh.i
    execute_on = 'INITIAL'
  []
[]

[Transfers]
  [h1_transfer_from_subapp]
    type = MultiAppMFEMShapeEvaluationTransfer
    source_variable = submesh_potential
    variable = potential
    from_multi_app = mfem_app
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/EmbeddedSubmesh
    vtk_format = ASCII
  []
[]

[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.msh
[]

[Problem]
  type = MFEMProblem
  solve = false
  numeric_type = complex
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[AuxVariables]
  [u]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[Executioner]
  type = MFEMSteady
[]

[MultiApps]
  [subapp]
    type = FullSolveMultiApp
    input_files = sub_complex.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppMFEMCopyTransfer
    source_variable = u
    variable = u
    from_multi_app = subapp
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/DiffusionComplex
    vtk_format = ASCII
  []
[]

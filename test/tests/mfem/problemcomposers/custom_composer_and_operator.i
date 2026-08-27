[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
  uniform_refine = 2
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = SECOND
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1
  []
[]

[Solvers]
  [main]
    type = MFEMMUMPS
  []
[]

[ProblemComposers]
  [default_steady]
    type = CustomProblemComposer
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/custom_composer_and_operator
    vtk_format = ASCII
  []
[]

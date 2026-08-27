[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
  uniform_refine = 2
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [prob_ex0p_h1]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = SECOND
  []
[]

[Variables]
  [prob_ex0p_var0]
    type = MFEMVariable
    fespace = prob_ex0p_h1
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

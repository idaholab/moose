[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
  dim = 2
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[ICs]
  [ic]
    type = MFEMScalarIC
    variable = concentration
    coefficient = -100.0
  []
[]

[BCs]
  [bc]
    type = MFEMScalarDirichletBC
    variable = concentration
    coefficient = 1
  []
[]

[Functions]
  [force]
    type = MFEMParsedFunction
    expression = if(sqrt(x^2+y^2)<1,v*sin(y*x),0)
    symbol_names = v
    symbol_values = concentration
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
  []
  [source]
    type = MFEMDomainLFKernel
    variable = concentration
    coefficient = force
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ParsedFunctionSource
    vtk_format = ASCII
  []
[]

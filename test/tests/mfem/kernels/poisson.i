[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
  dim = 2
  [MFEMPeriodic]
    periodic = true
    translation_x = "1. 0."
    translation_y = "0. 0."
  []
[]

[Problem]
  type = MFEMProblem
[]

[Functions]
  [HeatSourceFcn]
    type = ParsedVectorFunction
    expression_x = if(x>=0.75,10.0,0.0)
    expression_y = if(y>=0.75,1.0,0.0)
  []
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
  [concentration_gradient]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxKernels]
  [grad]
    type = MFEMGradAux
    variable = concentration_gradient
    source = concentration
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [top]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'top'
    coefficient = 0.0
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'bottom'
    coefficient = 0.0
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
    coefficient = 1.0
  []
  [HeatSource]
    type = MFEMVectorDomainLFKernel
    variable = concentration
    vector_coefficient = HeatSourceFcn
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  active = ParaViewDataCollection
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/poisson
    vtk_format = ASCII
  []
  [VisItDataCollection]
    type = MFEMVisItDataCollection
    file_base = OutputData/VisItDataCollection
  []
  [ConduitDataCollection]
    type = MFEMConduitDataCollection
    file_base = OutputData/ConduitDataCollection/Run
    protocol = conduit_bin
  []
[]

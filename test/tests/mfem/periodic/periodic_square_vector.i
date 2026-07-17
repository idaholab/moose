[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
  lattice_vectors = '1.0 0.0;
                     0.0 0.0'
[]

[Problem]
  type = MFEMProblem
[]

[Functions]
  [SourceFunction]
    type = ParsedFunction
    expression = if(x>=0.75,2.0,1.0)
  []
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

[BCs]
  [walls]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'top bottom'
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
    coefficient = SourceFunction
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solvers]
  [main]
    type = MFEMHypreGMRES
    preconditioner = boomeramg
    l_tol = 1e-16
    l_max_its = 1000
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [solution_l2_norm]
    type = MFEML2Error
    variable = concentration
    function = 0
  []
[]

[Outputs]
  csv = true
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/periodic_square_vector
    vtk_format = ASCII
  []
[]

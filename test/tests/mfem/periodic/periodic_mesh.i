[Mesh]
  type = MFEMMesh
  file = ../mesh/periodic-torus-sector.msh
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

[BCs]
  [exterior]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 3
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
    file_base = OutputData/PeriodicGmsh
    vtk_format = ASCII
  []
[]

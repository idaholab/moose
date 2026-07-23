[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
  []
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
  []
  [dg_domain]
    type = MFEMDomainLFKernel
    variable = concentration
  []
  [dg_diff]
    type = MFEMDGDiffusionKernel
    variable = concentration
  []
[]

[BCs]
  [dg_diff_bc]
    type = MFEMDGDiffusionBC
    variable = concentration
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solvers]
  [main]
    type = MFEMCGSolver
    preconditioner = boomeramg
    l_tol = 1e-16
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/DG_Diffusion
    vtk_format = ASCII
  []
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
  dim = 2
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = FIRST
    basis = GaussLegendre
    ordering = NODES
  []
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
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
    sigma = -1.0
  []
  [dg_diff_lf_bc]
    type = MFEMDGDirichletLFBC
    variable = concentration
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMCGSolver
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000
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

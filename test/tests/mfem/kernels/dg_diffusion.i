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


#[BCs]
#  [bottom]
#    type = MFEMScalarDirichletBC
#    variable = concentration
#    boundary = 'bottom'
#    coefficient = 1.0
#  []
#  [top]
#    type = MFEMScalarDirichletBC
#    variable = concentration
#    boundary = 'top'
#  []
#[]

[Kernels]
  [dg_diff]
    type = MFEMDGDiffusionKernel
    variable = concentration
  []
  [dg_diff_domain]
    type = MFEMDomainLFKernel
    variable = concentration
  []
  [dg_diff_bdr]
    type = MFEMDGDirichletLFKernel
    variable = concentration
  []
  [diff]
    type = MFEMDiffusionKernel
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
    file_base = OutputData/Diffusion
    vtk_format = ASCII
  []
[]

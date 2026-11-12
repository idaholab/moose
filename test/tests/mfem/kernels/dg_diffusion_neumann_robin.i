# DG diffusion on a periodic strip with two circular holes, with a Neumann condition
# du/dn = 1 on boundary 1, a Robin condition du/dn + u = 1 on boundary 2, a weakly imposed
# Dirichlet condition u = 0 on boundary 3, and the natural condition du/dn = 0 on boundary 4.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/periodic-holes.mesh
  uniform_refine = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = FIRST
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
  []
  [dg_diff]
    type = MFEMDGDiffusionKernel
    variable = u
  []
[]

[BCs]
  [neumann]
    type = MFEMBoundaryIntegratedBC
    variable = u
    boundary = 1
    coefficient = 1
  []
  [robin]
    type = MFEMConvectiveHeatFluxBC
    variable = u
    boundary = 2
    heat_transfer_coefficient = 1
    T_infinity = 1
  []
  [dirichlet]
    type = MFEMDGDiffusionBC
    variable = u
    boundary = 3
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-12
    l_max_its = 200
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [l2_norm]
    type = MFEML2Error
    variable = u
    function = 0
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = OutputData/DGDiffusionNeumannRobin
  []
[]

# Solves -div(grad u) = 1 with u = 0 on the boundary, discretized with the symmetric interior
# penalty DG method plus the stabilization of the second method of Bassi and Rebay (BR2), and
# records the L2 norm of the solution.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/star.mesh
  uniform_refine = 2
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = FIRST
    basis = GaussLegendre
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
  [source]
    type = MFEMDomainLFKernel
    variable = u
  []
  [dg_diff]
    type = MFEMDGDiffusionKernel
    variable = u
  []
  [br2]
    type = MFEMDGDiffusionBR2Kernel
    variable = u
    eta = 1
  []
[]

[BCs]
  [dg_diff]
    type = MFEMDGDiffusionBC
    variable = u
  []
  [br2]
    type = MFEMDGDiffusionBR2IntegratedBC
    variable = u
    eta = 1
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    print_level = 0
  []
  [main]
    type = MFEMCGSolver
    preconditioner = boomeramg
    l_tol = 1e-12
    l_max_its = 500
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
    file_base = OutputData/DGDiffusionBR2/dg_diffusion_br2
  []
[]

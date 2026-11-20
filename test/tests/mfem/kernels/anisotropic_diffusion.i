# Solves the Poisson problem -Div(sigma Grad u) = 1, in 3D on a 2D surface,
# where the diffusion coefficient sigma is a constant 3x3 matrix.
# Based on MFEM Example ex29p, with the function coefficient matrix changed to
# a constant matrix coefficient.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/crown.mesh
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = 3
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [dirichlet]
    type = MFEMScalarDirichletBC
    variable = u
    coefficient = 0.0
  []
[]

[Kernels]
  [linear_form]
    # Linear form b(u)
    type = MFEMDomainLFKernel
    variable = u
    coefficient = 1.0
  []
  [bilinear_form]
    # Bilinear form a(u,v): (Q \nabla u, \nabla v), where Q is the matrix coefficient.
    type = MFEMDiffusionKernel
    variable = u
    matrix_coefficient = '100.0 1.5 0.0; 1.5 2.5 0.0; 0.0 0.0 3.0'
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Solvers]
  [main]
    type = MFEMCGSolver
    preconditioner = boomeramg
    l_tol = 1e-12
  []
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Postprocessors]
  [u_l2_norm]
    type = MFEML2Error
    variable = u
    function = 0
  []
  [u_average]
    type = MFEMElementAverageValue
    variable = u
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = OutputData/AnisotropicDiffusion
  []
[]

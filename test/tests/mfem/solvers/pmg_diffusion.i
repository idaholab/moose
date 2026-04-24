# MMS verification of p-multigrid (geometric multigrid) preconditioner.
#
# PDE:  -∇²u = f  on [0,1]²
# Exact solution: u = sin(πx)sin(πy)
# Forcing:        f = 2π²sin(πx)sin(πy)
# BCs:            u = 0 on all four sides (exact solution is zero on ∂Ω)
#
# Multigrid hierarchy (two levels):
#   level 0 (coarse): H1 order 1
#   level 1 (fine):   H1 order 2  — variable lives here via fespace = h1_hierarchy
#
# Solvers:
#   coarse_solver = coarse     (CG + BoomerAMG on the coarse system)
#   smoothers     = chebyshev  (Chebyshev smoother at the fine level; SPD by construction)

[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
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

[FESpaceHierarchies]
  [h1_hierarchy]
    type = MFEMFESpaceHierarchy
    fespace = H1FESpace
    refinements = '2'
  []
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace_hierarchy = h1_hierarchy
  []
[]

[BCs]
  [zero]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'bottom right top left'
  []
[]

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = 'sin(pi*x)*sin(pi*y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '2*pi^2*sin(pi*x)*sin(pi*y)'
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
  []
  [rhs]
    type = MFEMDomainLFKernel
    variable = concentration
    coefficient = forcing
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    problem_solver = false
    print_level = 0
  []
  [coarse]
    type = MFEMCGSolver
    problem_solver = false
    preconditioner = boomeramg
    l_max_its = 10
    l_tol = 1e-2
    print_level = -1
  []
  [chebyshev]
    type = MFEMOperatorChebyshevSmoother
    problem_solver = false
    order = 2
  []
  [pmg]
    type = MFEMGeometricMultigridSolver
    problem_solver = false
    variable = concentration
    fespace_hierarchy = h1_hierarchy
    smoothers = 'chebyshev'
    coarse_solver = coarse
    assembly_levels = 'legacy'
  []
  [main]
    type = MFEMCGSolver
    preconditioner = pmg
    l_tol = 1e-10
    l_max_its = 200
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [l2_error]
    type = MFEML2Error
    variable = concentration
    function = u_exact
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = OutputData/PMGDiffusion
  []
[]

# Diffusion solve using a p-multigrid (geometric multigrid) preconditioner.
#
# PDE:  -Laplacian(u) = f  on [0,1]^2
# Exact solution: u = sin(pi x)sin(pi y)
# Forcing:        f = 2 pi^2 sin(pi x)sin(pi y)
# BCs:            u = 0 on all four sides (exact solution is zero on boundary)
#
# Multigrid hierarchy (two levels):
#   level 0 (coarse): H1 order 1
#   level 1 (fine):   H1 order 2  - variable lives here via fespace = h1_hierarchy
#
# Solvers:
#   coarse_solver = coarse     (CG + BoomerAMG on the coarse system)
#   smoothers     = chebyshev  (Chebyshev smoother at the fine level; SPD by construction)

[Mesh]
  type = MFEMFileMesh
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
    # One refinement entry adds the fine p=2 level, giving two total levels.
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
    print_level = 0
  []
  [coarse]
    type = MFEMCGSolver
    preconditioner = boomeramg
    l_max_its = 10
    l_tol = 1e-2
    print_level = -1
  []
  [chebyshev]
    type = MFEMOperatorChebyshevSmoother
    order = 2
  []
  [pmg]
    type = MFEMGeometricMultigridSolver
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

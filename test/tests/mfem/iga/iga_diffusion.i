# Isogeometric analysis (IGA) of a Poisson problem on a disc of radius sqrt(8),
# whose curved boundary is represented exactly by the NURBS geometry of the mesh.
#
#   -div(grad(u)) = 4    in the disc,
#               u = 0    on the boundary,
#
# which has the exact solution u = 8 - x^2 - y^2.

[Mesh]
  type = MFEMMesh
  file = ../mesh/disc-nurbs.mesh
  # Refinement of a NURBS mesh inserts knots, leaving the geometry unchanged.
  uniform_refine = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [NURBSFESpace]
    type = MFEMNURBSFESpace
    fec_order = SECOND
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = NURBSFESpace
  []
[]

[Functions]
  [exact_solution]
    type = ParsedFunction
    expression = '8 - x*x - y*y'
  []
[]

[BCs]
  [outer_boundary]
    type = MFEMScalarDirichletBC
    variable = u
  []
[]

[Kernels]
  [diffusion]
    type = MFEMDiffusionKernel
    variable = u
  []
  [source]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = 4.0
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-16
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [l2_error]
    type = MFEML2Error
    variable = u
    function = exact_solution
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/IGADiffusion
[]

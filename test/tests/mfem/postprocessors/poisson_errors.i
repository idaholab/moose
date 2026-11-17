# Poisson problem -Laplacian(u) = 2 pi^2 sin(pi x) sin(pi y) on the unit square with u = 0 on the
# boundary, whose exact solution is u = sin(pi x) sin(pi y), the errors of its finite element
# solution in several norms, and the L2 norms of the exact solution and its gradient.

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
    fec_order = SECOND
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [zero]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = 'bottom right top left'
  []
[]

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = 'sin(pi * x) * sin(pi * y)'
  []
  [grad_u_exact]
    type = ParsedVectorFunction
    expression_x = 'pi * cos(pi * x) * sin(pi * y)'
    expression_y = 'pi * sin(pi * x) * cos(pi * y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '2 * pi^2 * sin(pi * x) * sin(pi * y)'
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
  []
  [rhs]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = forcing
  []
[]

[Solvers]
  [superlu]
    type = MFEMSuperLU
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
    function = u_exact
  []
  [h1_error]
    type = MFEMH1Error
    variable = u
    function = u_exact
    gradient = grad_u_exact
  []
  [l1_error]
    type = MFEML1Error
    variable = u
    function = u_exact
  []
  [u_exact_norm]
    type = MFEMFunctionL2Norm
    function = u_exact
    quadrature_order = 5
  []
  [grad_u_exact_norm]
    type = MFEMVectorFunctionL2Norm
    function = grad_u_exact
    quadrature_order = 5
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = OutputData/PoissonErrors
  []
[]

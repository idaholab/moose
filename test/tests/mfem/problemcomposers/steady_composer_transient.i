# Steady Poisson problem -Laplacian(u) = 2 pi^2 (1 + t) sin(pi x) sin(pi y) on the unit square with
# u = t x on the boundary, whose exact solution is u = (1 + t) sin(pi x) sin(pi y) + t x, solved
# again at every step of a transient executioner, at the time at the end of the step.

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

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = '(1 + t) * sin(pi * x) * sin(pi * y) + t * x'
  []
  [forcing]
    type = ParsedFunction
    expression = '2 * pi^2 * (1 + t) * sin(pi * x) * sin(pi * y)'
  []
[]

[BCs]
  [boundary]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = 'bottom right top left'
    coefficient = u_exact
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

[WeakForms]
  [poisson]
    type = MFEMWeakForm
    kernels = 'diff rhs'
    bcs = 'boundary'
  []
[]

[ProblemComposers]
  [steady]
    type = MFEMWeakFormProblemComposer
    weak_form = poisson
  []
[]

[Solvers]
  [superlu]
    type = MFEMSuperLU
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  # The first step ends, and so solves, at t = 0
  start_time = -0.5
  dt = 0.5
  end_time = 1.0
[]

[Postprocessors]
  [l2_error]
    type = MFEML2Error
    variable = u
    function = u_exact
  []
  [u_norm]
    type = MFEML2Error
    variable = u
    function = 0
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = OutputData/SteadyComposerTransient
  []
[]

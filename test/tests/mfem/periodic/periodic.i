# Screened Poisson problem -div(grad u) + u = f on a periodic square with no boundary elements,
# with exact solution u = sin(pi x) sin(pi y).

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/periodic-square.mesh
  serial_refine = 2
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
  [source]
    type = ParsedFunction
    expression = '(1 + 2 * pi^2) * sin(pi * x) * sin(pi * y)'
  []
  [exact]
    type = ParsedFunction
    expression = 'sin(pi * x) * sin(pi * y)'
  []
[]

[Kernels]
  [diffusion]
    type = MFEMDiffusionKernel
    variable = u
  []
  [mass]
    type = MFEMMassKernel
    variable = u
  []
  [source]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = source
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    print_level = 0
  []
  [cg]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-12
    print_level = 0
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
    function = exact
  []
[]

[Outputs]
  [CSV]
    type = CSV
    file_base = OutputData/Periodic
  []
[]

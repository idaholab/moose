# Eigenproblem -div(grad u) + u = lambda u on a periodic square with no boundary elements, whose
# exact eigenvalues are 1 + pi^2 (k^2 + l^2) for integers k and l.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/periodic-square.mesh
  serial_refine = 2
  parallel_refine = 1
[]

[Problem]
  type = MFEMEigenproblem
  num_modes = 5
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
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
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    print_level = 0
  []
  [LOBPCG]
    type = MFEMHypreLOBPCG
    preconditioner = boomeramg
    print_level = 0
    l_tol = 1e-8
    l_max_its = 200
    random_seed = 75
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [eigenvalues]
    type = MFEMEigenvaluesPostprocessor
  []
[]

[Outputs]
  [CSV]
    type = CSV
    file_base = OutputData/PeriodicEigenproblem
  []
[]

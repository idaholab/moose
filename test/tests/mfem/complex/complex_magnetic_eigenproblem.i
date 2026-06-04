[Mesh]
  type = MFEMMesh
  file = ../mesh/inline-quad.mesh
  dim = 2
  serial_refine = 2
[]

[Problem]
  type = MFEMEigenproblem
  numeric_type = complex
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
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [all]
    type = MFEMComplexScalarDirichletBC
    variable = u
  []
[]

[Kernels]
  [magnetic]
    type = MFEMComplexKernel
    variable = u
    [RealComponent]
      type = MFEMDiffusionKernel
    []
    [ImagComponent]
      type = MFEMConvectionKernel
      vector_coefficient = '-2 0'
    []
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    print_level = 0
  []
[]

[Solver]
  type = MFEMHypreLOBPCG
  preconditioner = boomeramg
  print_level = 0
  l_tol = 1e-10
  l_max_its = 300
  random_seed = 75
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
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/ComplexMagneticEigenproblem
  []
[]

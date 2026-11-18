[Mesh]
  type = MFEMFileMesh
  file = ../mesh/ref-square.mesh
  uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [h1]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = h1
  []
[]

[Functions]
  [tensor]
    type = MFEMMatrixCoefficientFunction
    matrix = 'diagonal 0.25;
              0.5      diagonal'
  []
  [diagonal]
    type = ParsedFunction
    expression = '2+x+y+t'
  []
  [vector_only]
    type = ParsedVectorFunction
    expression_x = 'x'
    expression_y = 'y'
  []
  [matrix_only]
    type = MFEMMatrixCoefficientFunction
    matrix = '1'
  []
[]

[Kernels]
  [diffusion]
    type = MFEMDiffusionKernel
    variable = u
    matrix_coefficient = tensor
  []
  [source]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = 1
  []
[]

[BCs]
  [zero]
    type = MFEMScalarDirichletBC
    variable = u
    coefficient = 0
  []
[]

[Solvers]
  [main]
    type = MFEMHypreGMRES
    l_tol = 1e-12
    l_max_its = 100
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  assembly_level = legacy
[]

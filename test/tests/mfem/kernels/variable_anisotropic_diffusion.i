# Solves the Poisson problem -Div(sigma Grad u) = 1, in 3D on a 2D surface,
# where the diffusion coefficient sigma is a 3x3 matrix.
# Based on MFEM Example 29.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/ex29p.mesh
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
    type = MFEMDomainLFKernel
    variable = u
    coefficient = 1.0
  []
  [bilinear_form]
    type = MFEMDiffusionKernel
    variable = u
    matrix_coefficient = sigma
  []
[]

[Functions]
  [sigma]
    type = MFEMMatrixCoefficientFunction
    matrix = 'sigma_0_0  sigma_0_1        0.0;
              sigma_0_1  sigma_1_1        0.0;
                    0.0        0.0  sigma_2_2'
  []
  [a]
    type = ParsedFunction
    expression = '17.0 - 2.0 * x * (1.0 + x)'
  []
  [sigma_0_0]
    type = ParsedFunction
    expression = '0.5 + x * x * (8.0 / a - 0.5)'
    symbol_names = 'a'
    symbol_values = 'a'
  []
  [sigma_0_1]
    type = ParsedFunction
    expression = 'x * y * (8.0 / a - 0.5)'
    symbol_names = 'a'
    symbol_values = 'a'
  []
  [sigma_1_1]
    type = ParsedFunction
    expression = '0.5 * x * x + 8.0 * y * y / a'
    symbol_names = 'a'
    symbol_values = 'a'
  []
  [sigma_2_2]
    type = ParsedFunction
    expression = 'a / 32.0'
    symbol_names = 'a'
    symbol_values = 'a'
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
    l_max_its = 2000
    use_initial_guess = false
  []
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/VariableAnisotropicDiffusion
    vtk_format = ASCII
    high_order_output = true
  []
[]

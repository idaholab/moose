# Solves the Poisson problem -Div(sigma Grad u) = 1, in 3D on a 2D surface,
# where the diffusion coefficient sigma is a 3x3 matrix whose magnitude decays
# with time.
# Based on MFEM Example 29.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/ex29p-time-dep.mesh
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

[ICs]
  [initial_u]
    type = MFEMScalarIC
    variable = u
    coefficient = initial_u
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
  [time_derivative]
    type = MFEMTimeDerivativeMassKernel
    variable = u
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
    expression = '(0.5 + x * x * (8.0 / a - 0.5)) * exp(-5*t)'
    symbol_names = 'a'
    symbol_values = 'a'
  []
  [sigma_0_1]
    type = ParsedFunction
    expression = 'x * y * (8.0 / a - 0.5) * exp(-5*t)'
    symbol_names = 'a'
    symbol_values = 'a'
  []
  [sigma_1_1]
    type = ParsedFunction
    expression = '(0.5 * x * x + 8.0 * y * y / a) * exp(-5*t)'
    symbol_names = 'a'
    symbol_values = 'a'
  []
  [sigma_2_2]
    type = ParsedFunction
    expression = '(a / 32.0) * exp(-5*t)'
    symbol_names = 'a'
    symbol_values = 'a'
  []
  [initial_u]
    type = ParsedFunction
    expression = '(0.25*(2+x)-z)*(z+0.25*(2+x))'
  []
[]

[Executioner]
  type = MFEMTransient
  assembly_level = legacy
  scheme = implicit-euler
  start_time = 0
  end_time = 1
  dt = 0.05
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
    file_base = OutputData/TimeDependentAnisotropicDiffusion
    vtk_format = ASCII
    high_order_output = true
    execute_on = 'INITIAL TIMESTEP_END'
    show = 'u'
  []
[]

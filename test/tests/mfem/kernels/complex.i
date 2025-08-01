[Mesh]
  type = MFEMMesh
  file = ../mesh/inline-quad.mesh
  dim = 3
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

[Variables]
  [u]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [mu]
    type = ParsedFunction
    expression = 1.0
  []
  [epsilon]
    type = ParsedFunction
    expression = 1.0
  []
  [sigma]
    type = ParsedFunction
    expression = 20.0
  []
  [omega]
    type = ParsedFunction
    expression = 10.0
  []
  [kappa_r]
    type = ParsedFunction
    expression = 12.72019649514069
  []
  [kappa_i]
    type = ParsedFunction
    expression = -7.861513777574233
  []
  [u0_r]
    type = ParsedFunction
    expression = exp(y*kappa_i)*cos(z*kappa_r)
    symbol_names = 'kappa_r kappa_i'
    symbol_values = 'kappa_r kappa_i'
  []
  [u0_i]
    type = ParsedFunction
    expression = -exp(y*kappa_i)*sin(z*kappa_r)
    symbol_names = 'kappa_r kappa_i'
    symbol_values = 'kappa_r kappa_i'
  []
  [stiffnessCoef]
    type = ParsedFunction
    expression = 0.0
    #expression = 1.0/mu
    symbol_names = 'mu'
    symbol_values = 'mu'
  []
  [massCoef]
    type = ParsedFunction
    expression = -omega*omega*epsilon
    symbol_names = 'epsilon omega'
    symbol_values = 'epsilon omega'
  []
  [lossCoef]
    type = ParsedFunction
    expression = omega*sigma
    symbol_names = 'sigma omega'
    symbol_values = 'sigma omega'
  []
[]

[BCs]
  [dbc]
    type = MFEMComplexScalarDirichletBC
    variable = u
    boundary = '1 2 3 4'
    coefficient_real = u0_r
    coefficient_imag = u0_i
  []
[]

[Kernels]
  #[diffusion_complex]
  #  type = MFEMComplexKernel
  #  variable = u
  #  [real_part]
  #    variable = u
  #    type = MFEMDiffusionKernel
  #    coefficient = stiffnessCoef
  #  []
  #  [imag_part]
  #    variable = u
  #    type = MFEMDiffusionKernel
  #    coefficient = 1.0
  #  []
  #[]
  [mass_complex]
    type = MFEMComplexKernel
    variable = u
    [real_part]
      variable = u
      type = MFEMMassKernel
      coefficient = massCoef
    []
    [imag_part]
      variable = u
      type = MFEMMassKernel
      coefficient = lossCoef
    []
  []
[]

[Preconditioner]
  [jacobi]
    type = MFEMCGSolver
    l_tol = 1e-16
    l_max_its = 1000
  []
[]

[Solver]
  type = MFEMGMRESSolver
  preconditioner = jacobi
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  numeric_type = complex
  assembly_level = partial
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ComplexDiffusion
    vtk_format = ASCII
  []
[]

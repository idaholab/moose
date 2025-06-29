# Grad-div problem using method of manufactured solutions,
# based on MFEM Example 4.

[Mesh]
  type = MFEMMesh
  file = ../mesh/beam-tet.mesh
  dim = 3
  uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
    ordering = "vdim"
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[Variables]
  [F]
    type = MFEMVariable
    fespace = HDivFESpace
  []
[]

[AuxVariables]
  [divF]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[AuxKernels]
  [div]
    type = MFEMDivAux
    variable = divF
    source = F
    execute_on = TIMESTEP_END
  []
[]

[Functions]
  [f]
    type = ParsedVectorFunction
    expression_x = '(1. + 2*kappa * kappa) * cos(kappa * x) * sin(kappa * y)'
    expression_y = '(1. + 2*kappa * kappa) * cos(kappa * y) * sin(kappa * x)'
    expression_z = '0'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []
  [F_exact]
    type = ParsedVectorFunction
    expression_x = 'cos(kappa * x) * sin(kappa * y)'
    expression_y = 'cos(kappa * y) * sin(kappa * x)'
    expression_z = '0'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []
[]

[BCs]
  [dirichlet]
    type = MFEMVectorNormalDirichletBC
    variable = F
    boundary = '1 2 3'
    vector_coefficient = F_exact
  []
[]

[Kernels]
  [divdiv]
    type = MFEMDivDivKernel
    variable = F
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = F
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = F
    vector_coefficient = f
  []
[]

[Preconditioner]
  [ADS]
    type = MFEMHypreADS
    fespace = HDivFESpace
  []
[]

[Solver]
  type = MFEMCGSolver
  preconditioner = ADS
  l_tol = 1e-16
  l_max_its = 1000
  print_level = 2
[]

[Executioner]
  type = MFEMSteady
  device = "cpu"
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/GradDiv
    vtk_format = ASCII
  []
[]

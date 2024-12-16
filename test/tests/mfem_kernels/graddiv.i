# Grad-div problem using method of manufactured solutions,
# based on MFEM Example 4.

[Mesh]
  type = MFEMMesh
  file = gold/beam-tet.mesh
  dim = 3
  uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HDivFESpace]
    type = MFEMFESpace
    fec_type = RT
    fec_order = CONSTANT
    vdim = 1
    ordering = "vdim"
  []
[]

[Variables]
  [F]
    type = MFEMVariable
    fespace = HDivFESpace
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
    type = MFEMVectorNormalDirichletFunctionBC
    variable = F
    boundary = '1 2 3'
    function = F_exact
  []
[]

[Materials]
  [Beamium]
    type = MFEMGenericConstantMaterial
    prop_names = 'alpha beta'
    prop_values = '1.0 1.0'
    block = '1 2'
  []
[]

[Kernels]
  [divdiv]
    type = MFEMDivDivKernel
    variable = F
    coefficient = alpha
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = F
    coefficient = beta
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = F
    function = f
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

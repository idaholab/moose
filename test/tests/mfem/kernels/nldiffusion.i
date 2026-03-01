[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
  dim = 2
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
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]


[ICs]
  [diffused_ic]
    type = MFEMScalarIC
    coefficient = initial
    variable = concentration
  []
[]

[Functions]
  [one]
    type = ParsedFunction
    expression = 1.0
  []
  [initial]
    type = ParsedFunction
    expression = 2*y+1
  []
  [diffCoeff]
    type = MFEMParsedFunction
    expression = u
    symbol_names = 'u'
    symbol_values = 'concentration'
  []
[]

[BCs]
  [top]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'top'
    coefficient = 3.0
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'bottom'
    coefficient = 1.0
  []

[]

[Kernels]
  active = 'nl'
  [nl]
    type = MFEMNLDiffusionKernel
    variable = concentration
    coefficient = diffCoeff
  []
  [force]
    type = MFEMDomainLFKernel
    variable = concentration
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    print_level = 0
  []
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solver]
   type = MFEMHypreGMRES
   preconditioner = boomeramg
   print_level = 1
   l_tol = 1e-16
   l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  nl_max_its = 100
  nl_abs_tol = 1.0e-10
  nl_rel_tol = 1.0e-9
  print_level = 1
[]

[Outputs]
  active = ParaViewDataCollection
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/NLDiffusion
    vtk_format = ASCII
  []
[]

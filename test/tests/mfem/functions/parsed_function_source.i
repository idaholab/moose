[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
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
  [variable]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[ICs]
  [ic]
    type = MFEMScalarIC
    variable = variable
    coefficient = material
  []
[]

[BCs]
  [bc]
    type = MFEMScalarDirichletBC
    variable = variable
    boundary = skin
  []
[]

[FunctorMaterials]
  [material]
    type = MFEMGenericFunctorMaterial
    prop_names = material
    prop_values = -100
  []
[]

[Functions]
  [r]
    type = ParsedFunction
    expression = hypot(x,y)
  []
  [p]
    type = ParsedFunction
    expression = atan2(y,x)
  []
  [source]
    type = MFEMParsedFunction
    expression = v*sin(w*p)
    symbol_names = 'p w v'
    symbol_values = 'p 4 variable'
  []
  [solution]
    type = MFEMParsedFunction
    expression = if(r<=1,-c*sin(w*p)*(r^w-r^2)/(w^2-4),0)
    symbol_names = 'r p w c'
    symbol_values = 'r p 4 material'
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = variable
  []
  [source]
    type = MFEMDomainLFKernel
    variable = variable
    coefficient = source
    block = wire
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHyprePCG
  preconditioner = boomeramg
  l_tol = 1e-16
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [error]
    type = MFEML2Error
    variable = variable
    function = solution
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/ParsedFunctionSource
[]

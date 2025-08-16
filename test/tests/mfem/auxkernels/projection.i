[Mesh]
  type = MFEMMesh
  file = ../mesh/wire2d.e
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
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [Az]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
  [J]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [B]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[Kernels]
  [diffusion]
    type = MFEMDiffusionKernel
    variable = Az
  []
  [source]
    type = MFEMDomainLFKernel
    variable = Az
    coefficient = J_source
  []
[]

[AuxKernels]
  [J]
    type = MFEMScalarProjectAux
    coefficient = J_source
    variable = J
  []
  [B]
    type = MFEMVectorProjectAux
    vector_coefficient = B_analytical
    variable = B
  []
[]

[BCs]
  [essential]
    type = MFEMScalarDirichletBC
    variable = Az
    boundary = 1
    coefficient = 1
  []
[]

[FunctorMaterials]
  [J_wire]
    type = MFEMGenericFunctorMaterial
    prop_names = J_source
    prop_values = 8.0
    block = wire
  []
  [B_air]
    type = MFEMGenericFunctorVectorMaterial
    prop_names = B_analytical
    prop_values = B_air
    block = air
  []
  [B_wire]
    type = MFEMGenericFunctorVectorMaterial
    prop_names = B_analytical
    prop_values = B_wire
    block = wire
  []
[]

[Functions]
  [B_air]
    type = ParsedVectorFunction
    expression_x = -${FunctorMaterials/J_wire/prop_values}/2/(x^2+y^2)*y
    expression_y =  ${FunctorMaterials/J_wire/prop_values}/2/(x^2+y^2)*x
  []
  [B_wire]
    type = ParsedVectorFunction
    expression_x = -${FunctorMaterials/J_wire/prop_values}/2*y
    expression_y =  ${FunctorMaterials/J_wire/prop_values}/2*x
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
  l_max_its = 1000
  print_level = 2
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Projection
    vtk_format = ASCII
  []
[]

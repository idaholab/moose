[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
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
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
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
    fespace = L2FESpace
  []
  [GAz]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [GAz(copy)]
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
    type = MFEMScalarProjectionAux
    variable = J
    coefficient = J_source
  []
  [GAz]
    type = MFEMGradAux
    variable = GAz
    source = Az
  []
  [GAz(copy)]
    type = MFEMVectorProjectionAux
    variable = GAz(copy)
    vector_coefficient = GAz
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
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Projection
    vtk_format = ASCII
  []
[]

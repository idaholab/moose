[Mesh]
    type = MFEMMesh
    file = ../mesh/2D_wire_crossection.e
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
  #For compatible pairing H1 order p -> ND order p -> RT order p-1
  [RTFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
   [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
    basis = GaussLegendre
  []
[]

[Variables]
   [Az]
    type = MFEMVariable
    fespace = H1FESpace
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

[AuxVariables]
  [J]
    type = MFEMVariable
    fespace = L2FESpace
  []
  [gradAz]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [B]
    type = MFEMVariable
    fespace = RTFESpace
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
  [gradAz]
    type = MFEMGradAux
    variable = gradAz
    source = Az
  []
  [B_from_gradAz]
    type = MFEMNDtoRTAux
    variable = B
    nd_source = gradAz
    scale_factor = 1.0
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

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHyprePCG
  preconditioner = boomeramg
  l_tol = 1e-8
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'B_from_gradAz'
    start_point = '0 2 0'
    end_point = '0 -2 0'
    num_points = 10
  []
[]

[Executioner]
  type = MFEMSteady
[]

[Outputs]
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/2DMagnetostatic
  []
[]


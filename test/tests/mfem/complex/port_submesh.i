[Mesh]
  type = MFEMMesh
  file = ../mesh/waveguide.g
  serial_refine = 0
[]

[Problem]
  type = MFEMEigenproblem
  num_modes = 3
[]

[SubMeshes]
  [port]
    type = MFEMBoundarySubMesh
    boundary = '5'
  []
[]

[FESpaces]
  [SubMeshHCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
    submesh = port
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [E]
    type = MFEMVariable
    fespace = SubMeshHCurlFESpace
  []
[]

[AuxVariables]
  [TE10]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]


[BCs]
  [all]
    type = MFEMVectorTangentialDirichletBC
    variable = E
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = E
    coefficient = 1.0
  []
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = SubMeshHCurlFESpace
    print_level = 0
    singular = true
  []
[]

[Solver]
  type = MFEMHypreAME
  preconditioner = ams
  print_level = 1
  coefficient = 1.0
  l_tol = 1e-12
  l_max_its = 100
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Transfers]
  [submesh_transfer]
    type = MFEMSubMeshTransfer
    from_variable = E
    to_variable = TE10
    mode = 0
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/PortSubmesh
    submesh = port
  []
[]
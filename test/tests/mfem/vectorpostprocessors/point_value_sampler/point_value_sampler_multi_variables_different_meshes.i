[Mesh]
  type = MFEMFileMesh
  file = ../../mesh/cylinder-hex-q2.gen
[]

[Problem]
  type = MFEMProblem
[]

[SubMeshes]
  [wire]
    type = MFEMDomainSubMesh
    block = interior
  []
[]

[FESpaces]
  [SubMeshH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = wire
  []
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [parent_variable]
    type = MFEMVariable
    fespace = H1FESpace
  []

  [submesh_variable]
    type = MFEMVariable
    fespace = SubMeshH1FESpace
  []
[]

[VectorPostprocessors]
  [sample]
    type = MFEMVariablePointValueSampler
    variables = 'parent_variable submesh_variable'
    points = '0 0 0'
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

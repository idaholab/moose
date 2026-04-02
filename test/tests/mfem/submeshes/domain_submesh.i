[Mesh]
  type = MFEMMesh
  file = ../mesh/cylinder-hex-q2.gen
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
  [submesh_potential]
    type = MFEMVariable
    fespace = SubMeshH1FESpace
  []
[]

[BCs]
  [top]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = front
    coefficient = 1.0
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = back
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = submesh_potential
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-8
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'submesh_potential'
    start_point = '0 -0.5 -0.5'
    end_point = '0 0.5 0.5'
    num_points = 101
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/DomainPotential/domain_potential
  []
[]

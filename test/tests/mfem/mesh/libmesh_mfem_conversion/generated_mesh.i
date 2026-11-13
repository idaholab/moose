elem_type = HEX8
file_base = generated

[Mesh]
  # Activate 'delete' to keep only the right half of the mesh.
  inactive = 'delete'
  [generated]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = ${elem_type}
    # MFEM requires positive block and boundary IDs.
    subdomain_ids = 1
    boundary_id_offset = 1
  []
  [right_half]
    type = SubdomainBoundingBoxGenerator
    input = generated
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 1'
  []
  [delete]
    type = BlockDeletionGenerator
    input = right_half
    block = 1
  []
[]

[Problem]
  type = MFEMProblem
[]

[SubMeshes]
  [right_half]
    type = MFEMDomainSubMesh
    block = 2
  []
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [sides]
    type = MFEMScalarDirichletBC
    variable = u
    coefficient = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
  []
  [source]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = 2.0
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHypreGMRES
    preconditioner = boomeramg
    l_tol = 1e-16
    l_max_its = 1000
  []
[]

[Executioner]
  type = MFEMSteady
[]

[Outputs]
  [mesh]
    type = MFEMMeshOutput
    file_base = ${file_base}
  []
[]

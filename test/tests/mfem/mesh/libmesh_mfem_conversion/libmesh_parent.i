[Mesh]
  type = FileMesh
  file = ../square.e
[]

[Problem]
  type = MFEMProblem
  first_order_mesh = false
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[AuxVariables]
  [mfem_mesh_var] # The variable solved on the mesh read directly by MFEM
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Variables]
  [libmesh_mesh_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [sides]
    type = MFEMScalarDirichletBC
    variable = libmesh_mesh_var
    coefficient = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = libmesh_mesh_var
  []
  [source]
    type = MFEMDomainLFKernel
    variable = libmesh_mesh_var
    coefficient = 2.0
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
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  file_base = 'libmesh_to_mfem_mesh_quads'
  exodus = true
  csv = true
[]

[Postprocessors]
  [Difference]
    type = MFEML2Error
    variable = libmesh_mesh_var
    function = mfem_mesh_var
  []
[]

[MultiApps]
  [mfem_app]
    type = FullSolveMultiApp
    input_files = mfem_sub.i
    execute_on = 'INITIAL'
  []
[]

[Transfers]
  [transfer_from_mfem]
    type = MultiAppMFEMCopyTransfer
    source_variable = mfem_mesh_var
    variable = mfem_mesh_var
    from_multi_app = mfem_app
  []
[]

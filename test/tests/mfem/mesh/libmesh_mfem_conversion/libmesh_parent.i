elem_type = square

[Mesh]
  type = FileMesh
  file = ../${elem_type}.e
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
[]

[Outputs]
  [libMeshOutput]
    type = MFEMMeshOutput
    file_base = 'libmesh-mesh-${elem_type}'
    ordering = HILBERT
  []
[]

[MultiApps]
  [mfem_app]
    type = FullSolveMultiApp
    input_files = mfem_sub.i
    execute_on = 'INITIAL'
  []
[]

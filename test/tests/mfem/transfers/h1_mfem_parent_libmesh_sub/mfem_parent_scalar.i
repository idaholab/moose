[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.e
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
  [mfem_scalar_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [libmesh_scalar_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [sides]
    type = MFEMScalarDirichletBC
    variable = mfem_scalar_var
    coefficient = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = mfem_scalar_var
  []
  [source]
    type = MFEMDomainLFKernel
    variable = mfem_scalar_var
    coefficient = 2.0
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [jacobi]
    type = MFEMOperatorJacobiSmoother
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

[MultiApps]
  [libmesh_app]
    type = FullSolveMultiApp
    input_files = libmesh_sub_scalar.i
    execute_on = 'INITIAL'
  []
[]

[Transfers]
  [transfer_from_libmesh]
    type = MultiApplibMeshToMFEMShapeEvaluationTransfer
    source_variable = libmesh_scalar_var
    variable = libmesh_scalar_var
    from_multi_app = libmesh_app
  []
[]

[Postprocessors]
  [Difference]
    type = MFEML2Error
    variable = mfem_scalar_var
    function = libmesh_scalar_var
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  file_base = 'mfem_parent_libmesh_sub_scalar_quads'
  csv = true
[]

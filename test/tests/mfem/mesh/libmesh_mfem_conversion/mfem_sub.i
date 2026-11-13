elem_type = square

[Mesh]
  type = MFEMFileMesh
  file = ../${elem_type}.e
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
  [mfem_mesh_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [sides]
    type = MFEMScalarDirichletBC
    variable = mfem_mesh_var
    coefficient = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = mfem_mesh_var
  []
  [source]
    type = MFEMDomainLFKernel
    variable = mfem_mesh_var
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
  [MFEMOutput]
    type = MFEMMeshOutput
    file_base = 'mfem-mesh-${elem_type}'
    ordering = HILBERT
  []
[]

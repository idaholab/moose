[Mesh]
  type = MFEMMesh
  file = ref-segment.mesh
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
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [ends]
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
[]

[Preconditioner]
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solvers]
  [main]
    type = MFEMHypreGMRES
    preconditioner = jacobi
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  active = ''
[]

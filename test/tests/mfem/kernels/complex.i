[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
  dim = 3
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
  [concentration]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '1'
    coefficient = 1.0
  []
  [top]
    type = MFEMComplexIntegratedBC
    variable = concentration
    boundary = '1'
    [real_part]
      type = MFEMBoundaryIntegratedBC
      variable = concentration
      coefficient = 1.0
    []
    [imag_part]
      type = MFEMBoundaryIntegratedBC
      variable = concentration
      coefficient = 1.0
    []
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
    coefficient = 1.0
  []
[]

[ComplexKernels]
  [c_diff]
    type = MFEMComplexKernel
    variable = concentration
    [real_part]
      variable = concentration
      type = MFEMDiffusionKernel
      coefficient = 1.0
    []
    [imag_part]
      variable = concentration
      type = MFEMDiffusionKernel
      coefficient = 1.0
    []
  []
[]


[Preconditioner]
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solver]
  type = MFEMGMRESSolver
  preconditioner = jacobi
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  numeric_type = complex
  assembly_level = full
[]


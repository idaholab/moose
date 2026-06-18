[Mesh]
  [gen]
    type = MFEMGeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [h1]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = h1
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
  []
[]

[BCs]
  [left]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = 'left'
    coefficient = 0
  []
  [right]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = 'right'
    coefficient = 1
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
  l_tol = 1e-12
  l_max_its = 100
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [avg]
    type = MFEMElementAverageValue
    variable = u
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]

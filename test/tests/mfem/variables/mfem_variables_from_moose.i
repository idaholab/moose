[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
[]

[Variables]
  [scalar_var]
    family = LAGRANGE
    order = FIRST
  []
  [vector_var]
    family = LAGRANGE_VEC
    order = FIRST
  []
[]

[AuxVariables]
  [scalar_auxvar]
    family = MONOMIAL
    order = CONSTANT
  []
  [vector_auxvar]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = scalar_var
    boundary = '1'
    coefficient = 1.0
  []
  [top]
    type = MFEMScalarDirichletBC
    variable = scalar_var
    boundary = '2'
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = scalar_var
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
  type = MFEMTransient
  device = cpu
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/VariableSetupTest
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  zmax = 0
  elem_type = QUAD9
[]

[Variables]
  [./diffused]
    order = SECOND
  [../]
[]

[Kernels]
  active = 'diff'
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[DiracKernels]
  [./foo]
    variable = diffused
    type = ConstantPointSource
    value = 1
    point = '0.3 0.3 0.0'
  [../]
[]

[BCs]
  active = 'all'
  [./all]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom left right top'
    value = 0.0
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

[]

[Outputs]
  file_base = out_os
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
  [./oversample]
    type = Exodus
    oversample = true
    refinements = 4
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  # DiracKernels don't seem to work quite right in parallel with
  # ParallelMesh enabled (segfaults).  See #2125 for more details.
  distribution = serial
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./point_source]
    type = ConstantPointSource
    variable = u
    value = 1.0
    point = '0.2 0.3 0.4'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = -snes_mf_operator
[]

[Output]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]


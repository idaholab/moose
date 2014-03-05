#
# Coupling of time derivatives of aux variables is an error
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./cd]
    type = DotCouplingKernel
    variable = u
    v = av
  [../]
[]

[AuxVariables]
  [./av]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./ak]
    type = ConstantAux
    variable = av
    value = 10
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 1
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.1
  num_steps = 5
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]

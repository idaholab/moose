[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  [../]
  [lower_d]
    type = LowerDBlockFromSidesetGenerator
    input = square
    new_block_name = 'lower'
    sidesets = 'top right'
  []
[]

[Variables]
  [./u]
    block = 0
  [../]
  [lower]
    block = 'lower'
  []
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    block = 0
  [../]
[]

[NodalKernels]
  [time]
    type = TimeDerivativeNodalKernel
    variable = lower
    block = lower
  []
  [growth]
    type = ConstantRate
    rate = 1
    variable = lower
    block = lower
  []
[]

[BCs]
  [./dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'left bottom'
    value = 0
  [../]
  [./neumann]
    type = ADCoupledLowerValue
    variable = u
    boundary = 'right top'
    lower_d_var = lower
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]

###########################################################
# This is a simple test of the GPUKernel System.
# It solves the Laplacian equation on a small 2x2 grid.
# The "GPUDiffusion" kernel is used to calculate the
# residuals of the weak form of this operator. The
# "GPUBodyForce" kernel is used to apply a time-dependent
# volumetric source.
###########################################################

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[GPUKernels]
  [./diff]
    type = GPUDiffusion
    variable = u
  [../]
  [./bf]
    type = GPUBodyForce
    variable = u
    postprocessor = ramp
  [../]
[]

[Functions]
  [./ramp]
    type = ParsedFunction
    expression = 't'
  [../]
[]

[Postprocessors]
  [./ramp]
    type = FunctionValuePostprocessor
    function = ramp
    execute_on = linear
  [../]
[]

[GPUBCs]
  active = 'left right'

  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = GPUDirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.0
  end_time = 1.0

  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = bodyforce_out_gpu
  exodus = true
[]

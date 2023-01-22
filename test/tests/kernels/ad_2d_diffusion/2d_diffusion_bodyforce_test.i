###########################################################
# This is a simple test of the Kernel System.
# It solves the Laplacian equation on a small 2x2 grid.
# The "Diffusion" kernel is used to calculate the
# residuals of the weak form of this operator. The
# "BodyForce" kernel is used to apply a time-dependent
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

[Kernels]
  [./diff]
    type = ADDiffusion
    variable = u
  [../]
  [./bf]
    type = ADBodyForce
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

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
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
  file_base = bodyforce_out
  exodus = true
[]

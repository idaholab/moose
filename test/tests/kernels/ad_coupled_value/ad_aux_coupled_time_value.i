###########################################################
# This is a simple test of the ADCoupledTimeTest kernel.
# The expected solution for the variable v is
# v(x) = 1/2 * (x^2 + x)
###########################################################

[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./time_u]
    type = ADTimeDerivative
    variable = u
  [../]
  [./fn_u]
    type = BodyForce
    variable = u
    function = 1
  [../]
  [./time_v]
    type = ADCoupledTimeTest
    variable = v
    v = u
  [../]
  [./diff_v]
    type = ADDiffusion
    variable = v
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = v
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 1
  [../]
[]

[Preconditioning]
  [./fdp]
    type = FDP
    full = true
  []
[]

[Postprocessors]
  [./l2]
    type = ElementL2Error
    variable = v
    function = '1/2 * (x^2 + x)'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]

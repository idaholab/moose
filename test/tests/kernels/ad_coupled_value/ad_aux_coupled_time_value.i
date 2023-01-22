###########################################################
# This is a simple test of coupling an aux variable into the
# ADCoupledTimeDerivative kernel.
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
  [./v]
  [../]
[]

[AuxVariables]
  [./u]
  [../]
[]

[Functions]
  [./u]
    type = ParsedFunction
    expression = 't'
  [../]
[]

[AuxKernels]
  [./u]
    type = FunctionAux
    variable = u
    function = u
  [../]
[]

[Kernels]
  [./time_v]
    type = ADCoupledTimeDerivative
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
  [./smp]
    type = SMP
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

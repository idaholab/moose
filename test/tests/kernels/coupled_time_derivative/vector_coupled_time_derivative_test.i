###########################################################
# This is a simple test of the VectorCoupledTimeDerivative kernel.
# The expected solution for the vector variable v is
# v_x(x) = 1/2 * (x^2 + x)
# v_y(x) = 1/2 * (x^2 + x)
###########################################################

[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Variables]
  [./u]
    family = LAGRANGE_VEC
  [../]
  [./v]
    family = LAGRANGE_VEC
  [../]
[]

[Kernels]
  [./time_u]
    type = VectorTimeDerivative
    variable = u
  [../]
  [./fn_u]
    type = VectorBodyForce
    variable = u
    function_x = 1
    function_y = 1
  [../]
  [./time_v]
    type = VectorCoupledTimeDerivative
    variable = v
    v = u
  [../]
  [./diff_v]
    type = VectorDiffusion
    variable = v
  [../]
[]

[BCs]
  [./left]
    type = VectorDirichletBC
    variable = v
    boundary = 'left'
    values = '0 0 0'
  [../]
  [./right]
    type = VectorDirichletBC
    variable = v
    boundary = 'right'
    values = '1 1 0'
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

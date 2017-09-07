[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
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

[Postprocessors]
  [./L2_norm]
    type = ElementL2Norm
    variable = u
  [../]
  [./integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
  [../]
  [./direct_sum]
    type = ElementMomentSum
    variable = u
  [../]
  [./direct_sum_old]
    type = ElementMomentSum
    variable = u
    implicit = false
  [../]
  [./direct_sum_older]
    type = ElementMomentSum
    variable = u
    use_old = true
    implicit = false
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
[]

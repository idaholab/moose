[Mesh]
  file = square.e
  second_order = true
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./l2_lagrange]
    order = FIRST
    family = L2_LAGRANGE
  [../]
  [./l2_hierarchic]
    order = FIRST
    family = L2_HIERARCHIC
  [../]
  [./one]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  # Coupling of nonlinear to Aux
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = CoupledForce
    variable = u
    v = one
  [../]
[]

[AuxKernels]
  [./coupled_l2_lagrange]
    variable = l2_lagrange
    type = CoupledAux
    value = 2
    operator = +
    coupled = u
  [../]
  [./coupled_l2_hierarchic]
    variable = l2_hierarchic
    type = CoupledAux
    value = 2
    operator = +
    coupled = u
  [../]
  [./constant]
    variable = one
    type = ConstantAux
    value = 1
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
 [./int2_u]
   type = ElementL2Norm
   variable = u
 [../]
 [./int2_l2_lagrange]
   type = ElementL2Norm
   variable = l2_lagrange
 [../]
 [./int2_l2_hierarchic]
   type = ElementL2Norm
   variable = l2_hierarchic
 [../]
 [./int_u]
   type = ElementIntegralVariablePostprocessor
   variable = u
 [../]
 [./int_l2_lagrange]
   type = ElementIntegralVariablePostprocessor
   variable = l2_lagrange
 [../]
 [./int_l2_hierarchic]
   type = ElementIntegralVariablePostprocessor
   variable = l2_hierarchic
 [../]
[]

[Outputs]
  [./ex_out]
    type = Exodus
    file_base = l2elemaux
    output_initial = true
    elemental_as_nodal = true
    interval = 1
  [../]
  [./console]
    type = Console
    perf_log = true
  [../]
[]

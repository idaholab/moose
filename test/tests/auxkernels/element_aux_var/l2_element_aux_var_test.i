[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
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
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_l2_hierarchic]
    variable = l2_hierarchic
    type = CoupledAux
    value = 2
    operator = +
    coupled = u
    execute_on = 'initial timestep_end'
  [../]
  [./constant]
    variable = one
    type = ConstantAux
    value = 1
    execute_on = 'initial timestep_end'
  [../]
[]

[BCs]
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
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Postprocessors]
 [./int2_u]
   type = ElementL2Norm
   variable = u
   execute_on = 'initial timestep_end'
 [../]
 [./int2_l2_lagrange]
   type = ElementL2Norm
   variable = l2_lagrange
   execute_on = 'initial timestep_end'
 [../]
 [./int2_l2_hierarchic]
   type = ElementL2Norm
   variable = l2_hierarchic
   execute_on = 'initial timestep_end'
 [../]
 [./int_u]
   type = ElementIntegralVariablePostprocessor
   variable = u
   execute_on = 'initial timestep_end'
 [../]
 [./int_l2_lagrange]
   type = ElementIntegralVariablePostprocessor
   variable = l2_lagrange
   execute_on = 'initial timestep_end'
 [../]
 [./int_l2_hierarchic]
   type = ElementIntegralVariablePostprocessor
   variable = l2_hierarchic
   execute_on = 'initial timestep_end'
 [../]
[]

[Outputs]
  [./ex_out]
    type = Exodus
    file_base = l2elemaux
    elemental_as_nodal = true
  [../]
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./high_order]
    order = NINTH
    family = MONOMIAL
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
  [./coupled_high_order]
    variable = high_order
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
 [./int2_ho]
   type = ElementL2Norm
   variable = high_order
   execute_on = 'initial timestep_end'
 [../]
 [./int_u]
   type = ElementIntegralVariablePostprocessor
   variable = u
   execute_on = 'initial timestep_end'
 [../]
 [./int_ho]
   type = ElementIntegralVariablePostprocessor
   variable = high_order
   execute_on = 'initial timestep_end'
 [../]
[]

[Outputs]
  [./ex_out]
    type = Exodus
    file_base = ho
    elemental_as_nodal = true
  [../]
[]

[Mesh]
  file = square.e
  displacements = 'x_disp y_disp'
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[GlobalParams]
  use_displaced_mesh = true
[]

[AuxVariables]
  [./x_disp]
  [../]
  [./y_disp]
  [../]
  [./one]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./five]
    order = FIRST
    family = LAGRANGE
  [../]
  [./three]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./coupled_nine]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./coupled_fifteen]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./coupled]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./coupled_nl]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./coupled_grad_nl]
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
  # Simple Aux Kernel
  # Shows coupling of Element to Nodal
  # Shows coupling of Element to non-linear
  # Shows coupling of Element to non-linear grad
  [./constant]
    variable = one
    type = ConstantAux
    value = 1
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_nine]
    variable = coupled_nine
    type = CoupledAux
    value = 3
    operator = *
    coupled = three
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_three]
    variable = three
    type = CoupledAux
    value = 2
    operator = +
    coupled = one
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_fifteen]
    variable = coupled_fifteen
    type = CoupledAux
    value = 5
    operator = *
    coupled = three
    execute_on = 'initial timestep_end'
  [../]
  [./coupled]
    variable = coupled
    type = CoupledAux
    value = 2
    coupled = five
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_nl]
    variable = coupled_nl
    type = CoupledAux
    value = 2
    coupled = u
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_grad_nl]
    variable = coupled_grad_nl
    type = CoupledGradAux
    grad = '2 0 0'
    coupled = u
    execute_on = 'initial timestep_end'
  [../]
  [./five]
    type = ConstantAux
    variable = five
    boundary = '1 2'
    value = 5
    execute_on = 'initial timestep_end'
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
[]

[Postprocessors]
  [./constant]
    type = ElementIntegralVariablePostprocessor
    variable = one
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_nine]
    type = ElementIntegralVariablePostprocessor
    variable = coupled_nine
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_three]
    type = ElementIntegralVariablePostprocessor
    variable = three
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_fifteen]
    type = ElementIntegralVariablePostprocessor
    variable = coupled_fifteen
    execute_on = 'initial timestep_end'
  [../]
  [./coupled]
    type = ElementIntegralVariablePostprocessor
    variable = coupled
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_nl]
    type = ElementIntegralVariablePostprocessor
    variable = coupled_nl
    execute_on = 'initial timestep_end'
  [../]
  [./coupled_grad_nl]
    type = ElementIntegralVariablePostprocessor
    variable = coupled_grad_nl
    execute_on = 'initial timestep_end'
  [../]
  [./five]
    type = ElementIntegralVariablePostprocessor
    variable = five
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = displaced_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [one]
    order = CONSTANT
    family = MONOMIAL
  []
  [five]
    order = FIRST
    family = LAGRANGE
  []
  [three]
    order = CONSTANT
    family = MONOMIAL
  []
  [coupled_nine]
    order = CONSTANT
    family = MONOMIAL
  []
  [coupled_fifteen]
    order = CONSTANT
    family = MONOMIAL
  []
  [coupled]
    order = CONSTANT
    family = MONOMIAL
  []
  [coupled_nl]
    order = CONSTANT
    family = MONOMIAL
  []
  [coupled_grad_nl]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  # Coupling of nonlinear to Aux
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = CoupledForce
    variable = u
    v = one
  []
[]

[AuxKernels]
  # Simple Aux Kernel
  # Shows coupling of Element to Nodal
  # Shows coupling of Element to non-linear
  # Shows coupling of Element to non-linear grad
  [constant]
    variable = one
    type = ConstantAux
    value = 1
  []
  [coupled_nine]
    variable = coupled_nine
    type = CoupledAux
    value = 3
    operator = *
    coupled = three
  []
  [coupled_three]
    variable = three
    type = CoupledAux
    value = 2
    operator = +
    coupled = one
  []
  [coupled_fifteen]
    variable = coupled_fifteen
    type = CoupledAux
    value = 5
    operator = *
    coupled = three
  []
  [coupled]
    variable = coupled
    type = CoupledAux
    value = 2
    coupled = five
  []
  [coupled_nl]
    variable = coupled_nl
    type = CoupledAux
    value = 2
    coupled = u
  []
  [coupled_grad_nl]
    variable = coupled_grad_nl
    type = CoupledGradAux
    grad = '2 0 0'
    coupled = u
  []
  [five]
    type = ConstantAux
    variable = five
    boundary = '3 1'
    value = 5
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

[]

[Outputs]
  file_base = out
  [exodus]
    type = Exodus
    elemental_as_nodal = true
  []
[]

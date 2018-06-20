# This makes sure that aux kernels using coupled scalar variables that are
# executed on initial will use the initial condition set on the scalar variable

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./aux_scalar]
    order = FIRST
    family = SCALAR
  [../]
  [./coupled]
  [../]
[]

[ICs]
  [./aux_scalar_ic]
    type = ScalarConstantIC
    variable = aux_scalar
    value = 123
  [../]
[]

[AuxKernels]
  [./coupled]
    type = CoupledScalarAux
    variable = coupled
    coupled = aux_scalar
    execute_on = 'initial linear'
  [../]
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
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

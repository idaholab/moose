[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Functions]
  [./lin1_fn]
    type = ParsedFunction
    expression = t
  [../]
  [./lin2_fn]
    type = ParsedFunction
    expression = 't+1'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./aux_scalar]
    order = SECOND
    family = SCALAR
  [../]
  [./coupled]
  [../]
  [./coupled_1]
  [../]
[]

[ICs]
  [./aux_scalar_ic]
    variable = aux_scalar
    values = '1.2 4.3'
    type = ScalarComponentIC
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./coupled]
    type = CoupledScalarAux
    variable = coupled
    coupled = aux_scalar
  [../]
  [./coupled_1]
    # Coupling to the "1" component of an aux scalar
    type = CoupledScalarAux
    variable = coupled_1
    component = 1
    coupled = aux_scalar
  [../]
[]

[AuxScalarKernels]
  [./aux_scalar_k]
    type = FunctionScalarAux
    variable = aux_scalar
    function = 'lin1_fn lin2_fn'
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

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 4
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

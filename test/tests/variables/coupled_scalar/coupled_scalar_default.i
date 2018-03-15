[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./coupled]
    type = CoupledScalarAux
    variable = coupled
    # Using default value
  [../]
  [./coupled_1]
    # Coupling to the "1" component of an aux scalar
    type = CoupledScalarAux
    variable = coupled_1
    component = 1
    # Setting explicit default
    coupled = 3.14159
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
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[ICs]
  [./aux_scalar_ic]
    variable = aux_scalar
    values = '1.2 4.3'
    type = ScalarComponentIC
  [../]
[]

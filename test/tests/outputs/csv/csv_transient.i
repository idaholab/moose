[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./aux0]
    order = SECOND
    family = SCALAR
  [../]
  [./aux1]
    family = SCALAR
    initial_condition = 5
  [../]
  [./aux2]
    family = SCALAR
    initial_condition = 10
  [../]
  [./aux_sum]
    family = SCALAR
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxScalarKernels]
  [./sum_nodal_aux]
    type = SumNodalValuesAux
    variable = aux_sum
    sum_var = u
    nodes = '1 2 3 4 5'
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
  [./mid_point]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  verbose = true
[]

[Outputs]
  csv = true
[]

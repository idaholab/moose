[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [v][]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = ADTimeDerivative
    variable = u
  [../]
  [coupled]
    type = ADCoupledValueTest
    variable = u
    v = v
  []
  [v_diff]
    type = Diffusion
    variable = v
  []
[]

[DGKernels]
  [dummy]
    type = ADDGCoupledTest
    variable = u
    v = v
  []
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
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  [dof_map]
    type = DOFMap
    execute_on = 'initial'
  []
[]

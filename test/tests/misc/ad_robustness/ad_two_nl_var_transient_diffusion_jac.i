penalty=1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = FIRST
  [../]
  [v]
    family = MONOMIAL
    order = FIRST
  []
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
    type = PenaltyDirichletBC
    variable = u
    boundary = left
    value = 0
    penalty = ${penalty}
  [../]
  [./right]
    type = PenaltyDirichletBC
    variable = u
    boundary = right
    value = 1
    penalty = ${penalty}
  [../]
  [./left_v]
    type = PenaltyDirichletBC
    variable = v
    boundary = left
    value = 0
    penalty = ${penalty}
  [../]
  [./right_v]
    type = PenaltyDirichletBC
    variable = v
    boundary = right
    value = 1
    penalty = ${penalty}
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  dtmin = 0.1
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

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

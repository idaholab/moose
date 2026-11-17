[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
  [v]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diff_u]
    type = ADArrayDiffusion
    variable = u
  []
  [diff_v]
    type = ADArrayDiffusion
    variable = v
  []
  [reaction_v]
    type = ADArrayReaction
    variable = v
    reaction_coefficient = rc
  []
  [v_on_u]
    type = ADArrayCoupledValueGradient
    variable = u
    v = v
    coef = '1 0.5'
    direction = '1 2 0'
  []
  [u_on_v]
    type = ADArrayCoupledValueGradient
    variable = v
    v = u
    coef = '0.1 0.2'
    direction = '0 1 0'
  []
[]

[BCs]
  [left_u]
    type = ADArrayDirichletBC
    variable = u
    boundary = left
    values = '0 0'
  []
  [right_u]
    type = ADArrayDirichletBC
    variable = u
    boundary = right
    values = '1 2'
  []
  [left_v]
    type = ADArrayDirichletBC
    variable = v
    boundary = left
    values = '1 0'
  []
  [right_v]
    type = ADArrayDirichletBC
    variable = v
    boundary = right
    values = '0 1'
  []
[]

[Materials]
  [rc]
    type = ADGenericConstant2DArray
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]

[Mesh]
  type = GeneratedMesh
  nx = 5
  ny = 5
  dim = 2
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
  []
  [v]
    family = LAGRANGE_VEC
  []
[]

[Kernels]
  [time_u]
    type = VectorTimeDerivative
    variable = u
  []
  [fn_u]
    type = VectorBodyForce
    variable = u
    function_x = 1
    function_y = 1
  []
  [time_v]
    type = VectorCoupledTimeDerivative
    variable = v
    v = u
  []
  [diff_v]
    type = VectorDiffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = VectorDirichletBC
    variable = v
    boundary = 'left'
    values = '0 0 0'
  []
  [right]
    type = VectorDirichletBC
    variable = v
    boundary = 'right'
    values = '1 1 0'
  []
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'uv'
    [uv]
      splitting = 'u v'
      # Generally speaking, there are four types of splitting we could choose
      # <additive,multiplicative,symmetric_multiplicative,schur>
      splitting_type  = symmetric_multiplicative
    []
    [u]
      vars = 'u'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre preonly'
    []
    [v]
      vars = 'v'
      petsc_options_iname = '-pc_type -ksp_type'
      petsc_options_value = '     hypre  preonly'
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
